#include "d2_tree_parent.hpp"
#include <cmath>

namespace d2
{
    //
    // Parent Element
    //

    // Dynamic Iterator

    namespace internal
    {
        void DynamicIterator::increment(int cnt)
        {
            if (!_ptr.expired())
                _adaptor->increment(cnt, _ptr.lock());
        }
        void DynamicIterator::decrement(int cnt)
        {
            if (!_ptr.expired())
                _adaptor->decrement(cnt, _ptr.lock());
        }

        bool DynamicIterator::is_begin() const
        {
            if (_ptr.expired() || _adaptor == nullptr)
                return false;
            return _adaptor->is_begin(_ptr.lock());
        }
        bool DynamicIterator::is_end() const
        {
            if (_ptr.expired() || _adaptor == nullptr)
                return false;
            return _adaptor->is_end(_ptr.lock());
        }
        bool DynamicIterator::is_null() const
        {
            return _ptr.expired() || _adaptor == nullptr || _adaptor->is_null(_ptr.lock());
        }
        bool DynamicIterator::is_equal(DynamicIterator it) const
        {
            if ((it._ptr.expired() && it._ptr.expired()) ||
                (it._adaptor == nullptr && _adaptor == nullptr))
                return true;
            if (!it._ptr.expired() && !_ptr.expired() && it._ptr.lock() == _ptr.lock())
            {
                return _adaptor->is_equal(it._adaptor.get(), _ptr.lock());
            }
            return false;
        }

        TreeIter<> DynamicIterator::value() const
        {
            if (_ptr.expired() || _adaptor == nullptr)
                return nullptr;
            return _adaptor->value(_ptr.lock());
        }

        TreeIter<> DynamicIterator::operator->() const
        {
            return _adaptor->value(_ptr.lock());
        }
        Element& DynamicIterator::operator*() const
        {
            return *_adaptor->value(_ptr.lock());
        }

        bool DynamicIterator::operator==(const DynamicIterator& other) const
        {
            return (_ptr.expired() && other._ptr.expired()) ||
                   (_adaptor == nullptr && other._adaptor == nullptr) ||
                   (_adaptor != nullptr && other._adaptor != nullptr && !_ptr.expired() &&
                    !other._ptr.expired() && _adaptor->is_equal(other._adaptor.get(), _ptr.lock()));
        }
        bool DynamicIterator::operator!=(const DynamicIterator& other) const
        {
            return !operator==(other);
        }

        DynamicIterator& DynamicIterator::operator=(const DynamicIterator& copy)
        {
            _ptr = copy._ptr;
            _adaptor = copy._adaptor->clone();
            return *this;
        }
    } // namespace internal

    void ParentElement::_setparent_of(ptr ptr)
    {
        internal::ElementView::from(ptr).setparent(
            std::static_pointer_cast<ParentElement>(shared_from_this())
        );
    }
    void ParentElement::_insert_setstate(ptr ptr)
    {
        _setparent_of(ptr);
        ptr->initialize();
        ptr->setstate(Created, true);
        ptr->setstate(Embedded, getstate(Embedded));
        ptr->setstate(Swapped, getstate(Swapped));
        if (ptr->state() != state())
        {
            ptr->state()->set_root(std::static_pointer_cast<ParentElement>(shared_from_this()));
        }
        _signal_write(Masked);
    }
    void ParentElement::_extract_setstate(ptr ptr)
    {
        ptr->setstate(Embedded, false);
        if (ptr->state() != state())
            ptr->state()->set_root(nullptr);
        _signal_write(Masked);
    }
    void ParentElement::_remove_setstate(ptr ptr)
    {
        ptr->setstate(Created, false);
        ptr->setstate(Embedded, false);
        _signal_write(Masked);
    }

    int ParentElement::border_for(BorderType type, cptr ptr) const
    {
        return _get_border_impl(type, ptr);
    }

    int ParentElement::resolve_units(Unit value, cptr elem) const
    {
        const bool adjust = value.getmods() & Unit::Mods::Adjusted;
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
        const auto border_off = dims ? 0
                                     : (horiz ? (inv ? -border_right : border_left)
                                              : (inv ? -border_bottom : border_top));

        int result = 0;
        if ((value.getmods() & Unit::Center) && !dims)
        {
            const auto ww = elem->layout(Layout::Width);
            const auto pw = layout(Layout::Width);
            result =
                ((horiz
                      ? (layout(Layout::Width) - elem->layout(Layout::Width) - border_horizontal)
                      : (layout(Layout::Height) - elem->layout(Layout::Height) - border_vertical)) /
                 2) +
                border_off +
                resolve_units(
                    Unit(
                        value.raw(),
                        value.getunits(),
                        value.getmods() & ~Unit::Center | Unit::Relative
                    ),
                    elem
                );
        }
        else
        {
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
                result = resolve_units(cpy, elem);
                break;
            }
            case Unit::Px:
            {
                if (inv)
                {
                    if (dims)
                    {
                        result = (horiz ? (layout(Layout::Width) - border_horizontal)
                                        : (layout(Layout::Height) - border_vertical)) -
                                 val;
                    }
                    else
                    {
                        result =
                            ((horiz ? (layout(Layout::Width) - elem->layout(Layout::Width))
                                    : (layout(Layout::Height) - elem->layout(Layout::Height))) -
                             val) +
                            border_off;
                    }
                }
                else
                    result = val + border_off;
                break;
            }
            case Unit::Pv:
            {
                const auto input = context()->input();
                const auto [width, height] = input->screen_size();
                const auto mval = (inv ? (1 - val) : val) - dims;
                result = std::ceil((mval * (horiz ? width : height)) + border_off);
                break;
            }
            case Unit::Pc:
            {
                const auto mval = inv ? (1 - val) : val;
                result = std::ceil(
                    (mval * (horiz ? (layout(Layout::Width) - border_horizontal)
                                   : (layout(Layout::Height) - border_vertical))) +
                    border_off
                );
                break;
            }
            }
        }

        if (adjust)
        {
            if (horiz)
            {
                return result / 2;
            }
            else
            {
                return result * 2;
            }
        }
        return result;
    }
    void ParentElement::_signal_context_change_impl(write_flag type, unsigned int prop, ptr element)
    {
        foreach_internal(
            [&](ptr elem)
            {
                internal::ElementView::from(elem).signal_context_change_sub(type, prop, element);
                return true;
            }
        );
    }
    void ParentElement::_state_change_impl(State state, bool value)
    {
        if (state == Swapped || state == Display || state == Created || state == Embedded)
        {
            foreach_internal(
                [&](ptr elem)
                {
                    if (state == Embedded && elem->state() != this->state())
                    {
                        if (value)
                        {
                            elem->state()->set_root(
                                std::static_pointer_cast<ParentElement>(shared_from_this())
                            );
                        }
                        else
                            elem->state()->set_root(nullptr);
                    }
                    elem->setstate(state, value);
                    return true;
                }
            );
        }
    }
    void ParentElement::_layout_for_impl(enum Layout type, cptr elem) const
    {
        elem->override_layout(type, resolve_units(elem->internal_layout(type), elem));
    }

    void ParentElement::layout_for(enum Layout type, cptr elem) const
    {
        _layout_for_impl(type, elem);
    }

    bool ParentElement::empty() const
    {
        return _empty_impl();
    }
    bool ParentElement::exists(const std::string& name) const
    {
        return _exists_impl(name);
    }
    bool ParentElement::exists(ptr ptr) const
    {
        return _exists_impl(ptr);
    }

    std::size_t ParentElement::index_of(id id)
    {
        if (std::holds_alternative<std::size_t>(id))
            return std::get<std::size_t>(id);
        return _index_of_impl(id);
    }

    TreeIter<> ParentElement::at(id id) const
    {
        if (std::holds_alternative<ptr>(id))
            return std::get<ptr>(id);
        return _at_impl(id);
    }

    TreeIter<> ParentElement::create(ptr ptr)
    {
        return _create_impl(ptr);
    }
    TreeIter<> ParentElement::override(ptr ptr)
    {
        return _override_impl(ptr);
    }

    TreeIter<> ParentElement::create_after(id after, ptr p)
    {
        if (std::holds_alternative<ptr>(after))
            return _create_after_impl(p, std::get<ptr>(after));
        return _create_after_impl(p, after);
    }
    TreeIter<> ParentElement::override_after(id after, ptr p)
    {
        if (std::holds_alternative<ptr>(after))
            return _create_after_impl(p, std::get<ptr>(after));
        return _override_after_impl(p, after);
    }

    ParentElement::ptr ParentElement::extract(id id)
    {
        return _extract_impl(id);
    }

    void ParentElement::clear()
    {
        _clear_impl();
    }

    void ParentElement::remove(id id)
    {
        if (!_remove_impl(id))
            throw std::runtime_error{"Attempt to remove invalid object"};
    }
    bool ParentElement::remove_if(id id)
    {
        ;
        return _remove_impl(id);
    }

    //
    // VecParent
    //

    struct VecParentElement::LinearIteratorAdaptor
        : ParentElement::DynamicIterator::DynamicIteratorAdaptor
    {
        LinearIteratorAdaptor() = default;
        LinearIteratorAdaptor(const LinearIteratorAdaptor&) = default;
        LinearIteratorAdaptor(LinearIteratorAdaptor&&) = default;
        LinearIteratorAdaptor(std::vector<ptr>::iterator it) : current(it) {}

        std::vector<ParentElement::ptr>::iterator current{};

        auto ptr(std::shared_ptr<ParentElement> elem) const
        {
            return std::static_pointer_cast<VecParentElement>(elem);
        }

        TreeIter<> value(std::shared_ptr<ParentElement> elem) const
        {
            return *current;
        }
        void increment(std::shared_ptr<ParentElement> elem)
        {
            if (current != ptr(elem)->_elements.end())
                current++;
        }
        void decrement(std::shared_ptr<ParentElement> elem)
        {
            if (current != ptr(elem)->_elements.begin())
                current--;
        }
        bool is_null(std::shared_ptr<ParentElement> elem) const
        {
            return current.base() == nullptr;
        }
        bool is_begin(std::shared_ptr<ParentElement> elem) const
        {
            return current == ptr(elem)->_elements.begin();
        }
        bool is_end(std::shared_ptr<ParentElement> elem) const
        {
            return current == ptr(elem)->_elements.end();
        }
        bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const
        {
            return current == static_cast<LinearIteratorAdaptor*>(adapter)->current;
        }
        std::unique_ptr<DynamicIteratorAdaptor> clone() const
        {
            return std::make_unique<LinearIteratorAdaptor>(*this);
        }
    };

    // Implementation

    bool VecParentElement::_empty_impl() const
    {
        return _elements.empty();
    }
    bool VecParentElement::_exists_impl(const std::string& name) const
    {
        return _find(name) != ~0ull;
    }
    bool VecParentElement::_exists_impl(ptr ptr) const
    {
        return _find(ptr) != ~0ull;
    }

    std::size_t VecParentElement::_index_of_impl(id id) const
    {
        if (std::holds_alternative<ptr>(id))
            return _find(std::get<ptr>(id));
        return _find(std::get<std::string>(id));
    }

    TreeIter<> VecParentElement::_at_impl(id id) const
    {
        if (std::holds_alternative<std::size_t>(id))
        {
            const auto& idx = std::get<std::size_t>(id);
            if (idx >= _elements.size())
                throw std::runtime_error{
                    std::format("Attempt to reference invalid object at index: {}", idx)
                };
            return _elements[idx];
        }
        else if (std::holds_alternative<std::string>(id))
        {
            const auto& name = std::get<std::string>(id);
            const auto idx = _find(name);
            if (idx == ~0ull)
                throw std::runtime_error{
                    std::format("Attempt to reference invalid object: {}", name)
                };
            return {_elements[idx]};
        }
        return nullptr;
    }

    TreeIter<> VecParentElement::_create_impl(ptr ptr)
    {
        if (_find(ptr->name()) != ~0ull)
            throw std::runtime_error{"Attempt to create an object with a duplicate name"};
        auto result = _elements.emplace_back(ptr);
        _insert_setstate(ptr);
        return result;
    }
    TreeIter<> VecParentElement::_override_impl(ptr ptr)
    {
        const auto f = _find(ptr->name());
        if (f != ~0ull)
        {
            _remove_setstate(_elements[f]);
            _elements[f] = ptr;
            _insert_setstate(ptr);
            _signal_write(Style);
            return ptr;
        }
        _elements.emplace_back(ptr);
        _insert_setstate(ptr);
        return ptr;
    }

    TreeIter<> VecParentElement::_create_after_impl(ptr p, id after)
    {
        std::size_t idx = 0;
        if (std::holds_alternative<std::size_t>(after))
        {
            idx = std::get<std::size_t>(after) + 1;
            if (idx >= _elements.size())
                throw std::runtime_error{
                    std::format("Attempt to reference invalid object at index: {}", idx)
                };
        }
        else if (std::holds_alternative<std::string>(after))
        {
            const auto& name = std::get<std::string>(after);
            if (_find(p->name()) != ~0ull)
                throw std::runtime_error{"Attempt to create an object with a duplicate name"};
            idx = _find(name);
            if (idx == ~0ull)
                throw std::runtime_error{
                    "Attempt to create an object after a non-existent element"
                };
            idx++;
        }
        else if (std::holds_alternative<ptr>(after))
        {
            const auto& val = std::get<ptr>(after);
            if (val != nullptr)
            {
                if (_find(p->name()) != ~0ull)
                    throw std::runtime_error{"Attempt to create an object with a duplicate name"};
                idx = _find(val);
                if (idx == ~0ull)
                    throw std::runtime_error{
                        "Attempt to create an object after a non-existent element"
                    };
                idx++;
            }
        }
        _elements.insert(_elements.begin() + idx, p);
        _insert_setstate(p);
        return p;
    }
    TreeIter<> VecParentElement::_override_after_impl(ptr p, id after)
    {
        std::size_t idx = 0;
        if (std::holds_alternative<std::size_t>(after))
        {
            const auto& idx = std::get<std::size_t>(after) + 1;
            if (idx >= _elements.size())
                throw std::runtime_error{
                    std::format("Attempt to reference invalid object at index: {}", idx)
                };
        }
        else if (std::holds_alternative<std::string>(after))
        {
            const auto& name = std::get<std::string>(after);
            if (_find(p->name()) != ~0ull)
                throw std::runtime_error{"Attempt to create an object with a duplicate name"};
            idx = _find(name);
            if (idx == ~0ull)
                throw std::runtime_error{
                    "Attempt to create an object after a non-existent element"
                };
            idx++;
        }
        else if (std::holds_alternative<ptr>(after))
        {
            const auto& val = std::get<ptr>(after);
            if (val != nullptr)
            {
                if (_find(p->name()) != ~0ull)
                    throw std::runtime_error{"Attempt to create an object with a duplicate name"};
                idx = _find(val);
                if (idx == ~0ull)
                    throw std::runtime_error{
                        "Attempt to create an object after a non-existent element"
                    };
                idx++;
            }
        }
        if (idx == _elements.size() - 1)
            _elements.push_back(nullptr);
        else
            _remove_setstate(_elements[idx]);
        _elements[idx] = p;
        _insert_setstate(p);
        return p;
    }

    bool VecParentElement::_remove_impl(id id)
    {
        std::size_t idx = 0;
        if (std::holds_alternative<std::size_t>(id))
        {
            idx = std::get<std::size_t>(id);
            if (idx >= _elements.size())
                return false;
        }
        else if (std::holds_alternative<std::string>(id))
        {
            const auto& name = std::get<std::string>(id);
            idx = _find(name);
            if (idx >= _elements.size())
                return false;
        }
        else if (std::holds_alternative<ptr>(id))
        {
            const auto& val = std::get<ptr>(id);
            idx = _find(val);
            if (idx >= _elements.size())
                return false;
        }
        else
            return false;
        const auto ptr = _elements[idx];
        _remove_setstate(ptr);
        _elements.erase(_elements.begin() + idx);
        return true;
    }
    VecParentElement::ptr VecParentElement::_extract_impl(id id)
    {
        std::size_t idx = 0;
        if (std::holds_alternative<std::size_t>(id))
        {
            idx = std::get<std::size_t>(id);
            if (idx >= _elements.size())
                return nullptr;
        }
        else if (std::holds_alternative<std::string>(id))
        {
            const auto& name = std::get<std::string>(id);
            idx = _find(name);
            if (idx >= _elements.size())
                return nullptr;
        }
        else if (std::holds_alternative<ptr>(id))
        {
            const auto& val = std::get<ptr>(id);
            idx = _find(val);
            if (idx >= _elements.size())
                return nullptr;
        }
        else
            return nullptr;
        const auto ptr = _elements[idx];
        _extract_setstate(ptr);
        _elements.erase(_elements.begin() + idx);
        return ptr;
    }
    void VecParentElement::_clear_impl()
    {
        for (decltype(auto) it : _elements)
            _remove_setstate(it);
        _elements.clear();
        _elements.shrink_to_fit();
    }

    std::size_t VecParentElement::_find(const std::string& name) const
    {
        if (name.empty())
            return ~0ull;
        auto f = std::find_if(
            _elements.begin(),
            _elements.end(),
            [&name](const auto& elem) { return elem != nullptr && elem->name() == name; }
        );
        if (f == _elements.end())
            return ~0ull;
        return f - _elements.begin();
    }
    std::size_t VecParentElement::_find(ptr ptr) const
    {
        auto f = std::find(_elements.begin(), _elements.end(), ptr);
        if (f == _elements.end())
            return ~0ull;
        return f - _elements.begin();
    }

    VecParentElement::DynamicIterator VecParentElement::begin()
    {
        return DynamicIterator::make<LinearIteratorAdaptor>(
            std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.begin()
        );
    }
    VecParentElement::DynamicIterator VecParentElement::end()
    {
        return DynamicIterator::make<LinearIteratorAdaptor>(
            std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.end()
        );
    }
    void VecParentElement::foreach_internal(foreach_internal_callback callback) const
    {
        for (decltype(auto) it : _elements)
            if (!callback(it))
                break;
    }
    void VecParentElement::foreach (foreach_callback callback) const
    {
        for (decltype(auto) it : _elements)
            if (!callback(it->traverse()))
                break;
    }
} // namespace d2
