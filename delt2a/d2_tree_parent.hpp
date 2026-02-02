#ifndef D2_TREE_PARENT_HPP
#define D2_TREE_PARENT_HPP

#include "d2_tree_element.hpp"
#include <variant>

namespace d2
{
    class ParentElement : public Element
    {
    public:
        using DynamicIterator = internal::DynamicIterator;
        enum class BorderType
        {
            Top,
            Bottom,
            Left,
            Right
        };
        using id = std::variant<ptr, std::size_t, std::string>;
    protected:
        virtual void _layout_for_impl(enum Layout, cptr) const;

        virtual int _get_border_impl(BorderType, cptr) const { return 0; }
        virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) override;
        virtual void _state_change_impl(State state, bool value) override;

        virtual bool _empty_impl() const = 0;
        virtual bool _exists_impl(const std::string&) const = 0;
        virtual bool _exists_impl(ptr) const = 0;

        virtual TreeIter<> _at_impl(id id) const = 0;
        virtual TreeIter<> _create_impl(ptr ptr) = 0;
        virtual TreeIter<> _override_impl(ptr ptr) = 0;
        virtual TreeIter<> _create_after_impl(ptr p, id after) = 0;
        virtual TreeIter<> _override_after_impl(ptr p, id after) = 0;
        virtual bool _remove_impl(id id) = 0;
        virtual void _clear_impl() = 0;
    public:
        using Element::Element;

        bool empty() const;
        bool exists(const std::string& name) const;
        bool exists(ptr ptr) const;

        TreeIter<> at(id id) const;

        void layout_for(enum Layout layout, cptr elem)  const;

        int resolve_units(Unit, cptr) const;
        using Element::resolve_units;

        template<typename Type> TreeIter<Type> create(const std::string& name = "")
        {
            return create(Element::make<Type>(
                              name,
                              state(),
                              std::static_pointer_cast<ParentElement>(shared_from_this())
                          ));
        }
        template<typename Type> TreeIter<Type> override(const std::string& name = "")
        {
            return override(Element::make<Type>(
                                name,
                                state(),
                                std::static_pointer_cast<ParentElement>(shared_from_this())
                            ));
        }
        TreeIter<> create(ptr ptr);
        TreeIter<> override(ptr ptr);

        template<typename Type> TreeIter<Type> create_after(id after, const std::string& name = "")
        {
            return create_after(Element::make<Type>(
                name,
                state(),
                std::static_pointer_cast<ParentElement>(shared_from_this())
            ), after);
        }
        template<typename Type> TreeIter<Type> override_after(id after, const std::string& name = "")
        {
            return override_after(Element::make<Type>(
                name,
                state(),
                std::static_pointer_cast<ParentElement>(shared_from_this())
            ), after);
        }
        TreeIter<> create_after(ptr p, id after);
        TreeIter<> override_after(ptr p, id after);

        void clear();

        void remove(id id);
        bool remove_if(id id);

        int border_for(BorderType type, cptr ptr) const;

        virtual DynamicIterator begin() { return nullptr; }
        virtual DynamicIterator end() { return nullptr; }
        virtual void foreach_internal(foreach_internal_callback callback) const = 0;
        virtual void foreach(foreach_callback callback) const = 0;
    };
    class VecParentElement : public ParentElement
    {
    private:
        class LinearIteratorAdaptor;
    protected:
        std::vector<ptr> _elements{};

        virtual bool _empty_impl() const override;
        virtual bool _exists_impl(const std::string& name) const override;
        virtual bool _exists_impl(ptr ptr) const override;

        virtual TreeIter<> _at_impl(id id) const override;
        virtual TreeIter<> _create_impl(ptr ptr) override;
        virtual TreeIter<> _override_impl(ptr ptr) override;
        virtual TreeIter<> _create_after_impl(ptr p, id after) override;
        virtual TreeIter<> _override_after_impl(ptr p, id after) override;
        virtual bool _remove_impl(id id) override;
        virtual void _clear_impl() override;

        std::size_t _find(const std::string& name) const;
        std::size_t _find(ptr ptr) const;
    public:
        using ParentElement::ParentElement;

        virtual DynamicIterator begin() override;
        virtual DynamicIterator end() override;
        virtual void foreach_internal(foreach_internal_callback callback) const override;
        virtual void foreach(foreach_callback callback) const override;
    };
    class MetaParentElement : public ParentElement
    {
    protected:
        virtual bool _empty_impl() const override
        {
            return true;
        }
        virtual bool _exists_impl(const std::string& name) const override
        {
            return false;
        }
        virtual bool _exists_impl(ptr ptr) const override
        {
            return false;
        }

        virtual TreeIter<> _at_impl(id id) const override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter<> _create_impl(ptr ptr) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter<> _override_impl(ptr ptr) override
        {
            return nullptr;
        }
        virtual TreeIter<> _create_after_impl(ptr p, id after) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter<> _override_after_impl(ptr p, id after) override
        {
            throw std::logic_error{ "Not implemented" };
        }

        virtual bool _remove_impl(id id) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual void _clear_impl() override {}
    public:
        using ParentElement::ParentElement;

        virtual void foreach_internal(foreach_internal_callback callback) const override {}
        virtual void foreach (foreach_callback callback) const override {}
    };
}

#endif // D2_TREE_PARENT_HPP
