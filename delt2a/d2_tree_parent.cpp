#include "d2_tree_parent.hpp"
#include <cmath>

namespace d2
{
    //
    // Parent Element
    //

    // Dynamic Iterator

    void ParentElement::DynamicIterator::increment(int cnt) noexcept
    {
        if (!_ptr.expired())
            _adaptor->increment(cnt, _ptr.lock());
    }
    void ParentElement::DynamicIterator::decrement(int cnt) noexcept
    {
        if (!_ptr.expired())
            _adaptor->decrement(cnt, _ptr.lock());
    }

    bool ParentElement::DynamicIterator::is_begin() const noexcept
    {
        if (_ptr.expired() || _adaptor == nullptr)
            return false;
        return _adaptor->is_begin(_ptr.lock());
    }
    bool ParentElement::DynamicIterator::is_end() const noexcept
    {
        if (_ptr.expired() || _adaptor == nullptr)
            return false;
        return _adaptor->is_end(_ptr.lock());
    }
    bool ParentElement::DynamicIterator::is_null() const noexcept
    {
        return
            _ptr.expired() ||
            _adaptor == nullptr ||
            _adaptor->is_null(_ptr.lock());
    }
    bool ParentElement::DynamicIterator::is_equal(DynamicIterator it) const noexcept
    {
        if ((it._ptr.expired() && it._ptr.expired()) ||
                (it._adaptor == nullptr && _adaptor == nullptr))
            return true;
        if (!it._ptr.expired() && !_ptr.expired() &&
                it._ptr.lock() == _ptr.lock())
        {
            return _adaptor->is_equal(
                       it._adaptor.get(), _ptr.lock()
                   );
        }
        return false;
    }

    Element::ptr ParentElement::DynamicIterator::value() const noexcept
    {
        if (_ptr.expired() || _adaptor == nullptr)
            return nullptr;
        return _adaptor->value(_ptr.lock());
    }

    Element::ptr ParentElement::DynamicIterator::operator->() const noexcept
    {
        return _adaptor->value(_ptr.lock());
    }
    Element& ParentElement::DynamicIterator::operator*() const noexcept
    {
        return *_adaptor->value(_ptr.lock());
    }

    bool ParentElement::DynamicIterator::operator==(const DynamicIterator& other) const noexcept
    {
        return
            (_ptr.expired() && other._ptr.expired()) ||
            (_adaptor == nullptr && other._adaptor == nullptr) ||
            (_adaptor != nullptr && other._adaptor != nullptr &&
             !_ptr.expired() && !other._ptr.expired() &&
             _adaptor->is_equal(other._adaptor.get(), _ptr.lock()));
    }
    bool ParentElement::DynamicIterator::operator!=(const DynamicIterator& other) const noexcept
    {
        return !operator==(other);
    }

    ParentElement::DynamicIterator& ParentElement::DynamicIterator::operator=(const DynamicIterator& copy) noexcept
    {
        _ptr = copy._ptr;
        _adaptor = copy._adaptor->clone();
        return *this;
    }

    int ParentElement::border_for(BorderType type, cptr ptr) const noexcept
    {
        return _get_border_impl(type, ptr);
    }

    int ParentElement::resolve_units(Unit value, cptr elem) const noexcept
    {
        const bool dims = value.getmods() & UnitContext::Dimensions;
        const bool horiz = value.getmods() & UnitContext::Horizontal;
        const bool inv = value.getmods() & Unit::Mods::Inverted;
        const bool rel = value.getmods() & Unit::Relative;
        const float val = value.raw();

        const auto border_top = _get_border_impl(BorderType::Top, elem) * (!rel || dims);
        const auto border_bottom = _get_border_impl(BorderType::Bottom, elem) * (!rel || dims);
        const auto border_left = _get_border_impl(BorderType::Left, elem) * (!rel || dims);
        const auto border_right = _get_border_impl(BorderType::Right, elem) * (!rel || dims);
        const auto border_horizontal = border_left + border_right;
        const auto border_vertical = border_top + border_bottom;
        const auto border_off = dims ? 0 : (horiz ?
                                 (inv ? -border_right : border_left) :
                                 (inv ? -border_bottom : border_top));

        if (value.getmods() & Unit::Center)
        {
            if (dims)
                return 0;
            return ((horiz ?
                         (layout(Layout::Width) - elem->layout(Layout::Width) - border_horizontal) :
                         (layout(Layout::Height) - elem->layout(Layout::Height) - border_vertical)) / 2) +
                   border_off + resolve_units(Unit(value.raw(), value.getunits(), value.getmods() & ~Unit::Center), elem);
        }

        switch (value.getunits())
        {
            case Unit::Auto:
            {
                Unit cpy = value;
                if (dims)
                {
                    cpy.setunits(Unit::Pc);
                    cpy = 1.f - val;
                }
                else
                {
                    cpy.setunits(Unit::Px);
                }
                return resolve_units(cpy, elem);
            }
            case Unit::Px:
            {
                if (inv)
                {
                    if (dims) return (horiz ?
                        (layout(Layout::Width) - border_horizontal) :
                        (layout(Layout::Height) - border_vertical)) - val;
                    else return ((horiz ?
                                      (layout(Layout::Width) - elem->layout(Layout::Width)) :
                                      (layout(Layout::Height) - elem->layout(Layout::Height))) -
                                     val) + border_off;
                }
                else
                    return val + border_off;
            }
            case Unit::Pv:
            {
                const auto input = context()->input();
                const auto [ width, height ] = input->screen_size();
                const auto mval = (inv ? (1 - val) : val) - dims;
                return std::ceil((mval * (horiz ? width : height)) + border_off);
            }
            case Unit::Pc:
            {
                const auto mval = inv ? (1 - val) : val;
                return std::ceil((mval * (horiz ?
                                          (layout(Layout::Width) - border_horizontal) :
                                          (layout(Layout::Height) - border_vertical))) + border_off);
            }
        }
    }
    void ParentElement::_signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept
    {
        foreach_internal([&](ptr elem)
        {
            internal::ElementView::from(elem)
                .signal_context_change_sub(type, prop, element);
        });
    }
    void ParentElement::_state_change_impl(State state, bool value)
    {
        if (state == State::Swapped || state == State::Display)
        {
            foreach_internal([&](ptr elem)
            {
                elem->setstate(state, value);
            });
        }
    }
    void ParentElement::_layout_for_impl(enum Layout type, cptr elem) const
    {
        elem->override_layout(type, resolve_units(elem->internal_layout(type), elem));
    }

    void ParentElement::layout_for(enum Layout type, cptr elem) const noexcept
    {
        _layout_for_impl(type, elem);
    }

    //
    // VecParent
    //

    struct VecParentElement::LinearIteratorAdaptor :
        ParentElement::DynamicIterator::DynamicIteratorAdaptor
    {
        LinearIteratorAdaptor() = default;
        LinearIteratorAdaptor(const LinearIteratorAdaptor&) = default;
        LinearIteratorAdaptor(LinearIteratorAdaptor&&) = default;
        LinearIteratorAdaptor(std::vector<ptr>::iterator it) : current(it) {}

        std::vector<ParentElement::ptr>::iterator current{};

        auto ptr(std::shared_ptr<ParentElement> elem) const noexcept
        {
            return std::static_pointer_cast<VecParentElement>(elem);
        }

        Element::ptr value(std::shared_ptr<ParentElement> elem) const noexcept
        {
            return *current;
        }
        void increment(std::shared_ptr<ParentElement> elem) noexcept
        {
            if (current != ptr(elem)->_elements.end())
                current++;
        }
        void decrement(std::shared_ptr<ParentElement> elem) noexcept
        {
            if (current != ptr(elem)->_elements.begin())
                current--;
        }
        bool is_null(std::shared_ptr<ParentElement> elem) const noexcept
        {
            return current.base() == nullptr;
        }
        bool is_begin(std::shared_ptr<ParentElement> elem) const noexcept
        {
            return current == ptr(elem)->_elements.begin();
        }
        bool is_end(std::shared_ptr<ParentElement> elem) const noexcept
        {
            return current == ptr(elem)->_elements.end();
        }
        bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const noexcept
        {
            return current == static_cast<LinearIteratorAdaptor*>(adapter)->current;
        }
        std::unique_ptr<DynamicIteratorAdaptor> clone() const noexcept
        {
            return std::make_unique<LinearIteratorAdaptor>(*this);
        }
    };

    // Implementation

    bool VecParentElement::_empty_impl() const noexcept
    {
        return _elements.empty();
    }
    bool VecParentElement::_exists_impl(const std::string& name) const noexcept
    {
        return _find(name) != ~0ull;
    }
    bool VecParentElement::_exists_impl(ptr ptr) const noexcept
    {
        return _find(ptr) != ~0ull;
    }

    const TreeIter VecParentElement::_at_impl(const std::string& name) const
    {
        const auto idx = _find(name);
        if (idx == ~0ull)
            throw std::runtime_error{ std::format("Attempt to reference invalid object: {}", name) };
        return { _elements[idx] };
    }
    TreeIter VecParentElement::_create_impl(ptr ptr)
    {
        if (_find(ptr->name()) != ~0ull)
            throw std::runtime_error{ "Attempt to create an object with a duplicate name" };
        auto result = _elements.emplace_back(ptr);
        _elements.back()->setstate(Swapped, false);
        _elements.back()->initialize();
        _signal_write(Style);
        return result;
    }
    TreeIter VecParentElement::_override_impl(ptr ptr) noexcept
    {
        const auto f = _find(ptr->name());
        if (f != ~0ull)
        {
            _elements[f] = ptr;
            ptr->setstate(Swapped, false);
            _signal_write();
            return ptr;
        }
        _elements.emplace_back(ptr);
        ptr->setstate(Swapped, false);
        _elements.back()->initialize();
        _signal_write(Style);
        return ptr;
    }
    void VecParentElement::_remove_impl(const std::string& name)
    {
        const auto idx = _find(name);
        if (idx >= _elements.size())
            throw std::runtime_error{ "Attempt to remove invalid object" };
        _elements[idx]->setstate(Swapped);
        _elements.erase(_elements.begin() + idx);
        _signal_write(Style);
    }
    void VecParentElement::_remove_impl(ptr ptr)
    {
        const auto idx = _find(ptr);
        if (idx >= _elements.size())
            throw std::runtime_error{ "Attempt to remove invalid object" };
        _elements[idx]->setstate(Swapped);
        _elements.erase(_elements.begin() + idx);
        _signal_write(Style);
    }
    void VecParentElement::_clear_impl() noexcept
    {
        for (decltype(auto) it : _elements)
        {
            it->setstate(Swapped);
        }
        _elements.clear();
        _elements.shrink_to_fit();
        _signal_write(Style);
    }

    std::size_t VecParentElement::_find(const std::string& name) const noexcept
    {
        if (name.empty()) return ~0ull;
        auto f = std::find_if(_elements.begin(), _elements.end(), [&name](const auto& elem)
        {
            return elem != nullptr && elem->name() == name;
        });
        if (f == _elements.end())
            return ~0ull;
        return f - _elements.begin();
    }
    std::size_t VecParentElement::_find(ptr ptr) const noexcept
    {
        auto f = std::find(_elements.begin(), _elements.end(), ptr);
        if (f == _elements.end())
            return ~0ull;
        return f - _elements.begin();
    }

    // Public interface

    VecParentElement::DynamicIterator VecParentElement::begin() noexcept
    {
        return DynamicIterator::make<LinearIteratorAdaptor>(
                   std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.begin()
               );
    }
    VecParentElement::DynamicIterator VecParentElement::end() noexcept
    {
        return DynamicIterator::make<LinearIteratorAdaptor>(
                   std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.end()
               );
    }
    void VecParentElement::foreach_internal(foreach_internal_callback callback) const
    {
        for (decltype(auto) it : _elements)
            callback(it);
    }
    void VecParentElement::
    foreach (foreach_callback callback) const
    {
        for (decltype(auto) it : _elements)
            callback(it->traverse());
    }
}
