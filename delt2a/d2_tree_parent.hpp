#ifndef D2_TREE_PARENT_HPP
#define D2_TREE_PARENT_HPP

#include "d2_tree_element.hpp"

namespace d2
{
    class ParentElement : public Element
    {
    public:
        class DynamicIterator
        {
        public:
            struct DynamicIteratorAdaptor
            {
                DynamicIteratorAdaptor() = default;
                DynamicIteratorAdaptor(const DynamicIteratorAdaptor&) = default;
                DynamicIteratorAdaptor(DynamicIteratorAdaptor&&) = default;
                virtual ~DynamicIteratorAdaptor() = default;

                void increment(int cnt, std::shared_ptr<ParentElement> elem) noexcept
                {
                    for (std::size_t i = 0; i < cnt; i++)
                        increment(elem);
                }
                void decrement(int cnt, std::shared_ptr<ParentElement> elem) noexcept
                {
                    for (std::size_t i = 0; i < cnt; i++)
                        decrement(elem);
                }

                virtual ptr value(std::shared_ptr<ParentElement> elem) const noexcept = 0;
                virtual void increment(std::shared_ptr<ParentElement> elem) noexcept = 0;
                virtual void decrement(std::shared_ptr<ParentElement> elem) noexcept = 0;
                virtual bool is_null(std::shared_ptr<ParentElement> elem) const noexcept = 0;
                virtual bool is_begin(std::shared_ptr<ParentElement> elem) const noexcept = 0;
                virtual bool is_end(std::shared_ptr<ParentElement> elem) const noexcept = 0;
                virtual bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const noexcept = 0;
                virtual std::unique_ptr<DynamicIteratorAdaptor> clone() const noexcept = 0;
            };
        private:
            std::weak_ptr<ParentElement> _ptr{};
            std::unique_ptr<DynamicIteratorAdaptor> _adaptor{ nullptr };
        public:
            template<typename Adaptor, typename... Argv>
            static auto make(std::weak_ptr<ParentElement> ptr, Argv&&... args)
            {
                return DynamicIterator(
                           ptr, std::make_unique<Adaptor>(std::forward<Argv>(args)...)
                       );
            }

            DynamicIterator() = default;
            DynamicIterator(std::nullptr_t) {}
            DynamicIterator(const DynamicIterator& copy) :
                _ptr(copy._ptr), _adaptor(copy._adaptor->clone()) {}
            DynamicIterator(DynamicIterator&&) = default;
            DynamicIterator(std::weak_ptr<ParentElement> ptr, std::unique_ptr<DynamicIteratorAdaptor> adaptor) :
                _ptr(ptr), _adaptor(std::move(adaptor)) {}

            void increment(int cnt = 1) noexcept;
            void decrement(int cnt = 1) noexcept;

            bool is_begin() const noexcept;
            bool is_end() const noexcept;
            bool is_null() const noexcept;
            bool is_equal(DynamicIterator it) const noexcept;

            Element::ptr value() const noexcept;

            Element::ptr operator->() const noexcept;
            Element& operator*() const noexcept;

            bool operator==(const DynamicIterator& other) const noexcept;
            bool operator!=(const DynamicIterator& other) const noexcept;

            DynamicIterator& operator=(const DynamicIterator& copy) noexcept;
            DynamicIterator& operator=(DynamicIterator&&) noexcept = default;
        };
        enum class BorderType
        {
            Top,
            Bottom,
            Left,
            Right
        };
    protected:

        virtual void _layout_for_impl(enum Layout, cptr) const;

        virtual int _get_border_impl(BorderType, cptr) const noexcept { return 0; }
        virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept override;
        virtual void _state_change_impl(State state, bool value) override;

        virtual bool _empty_impl() const noexcept = 0;
        virtual bool _exists_impl(const std::string&) const noexcept = 0;
        virtual bool _exists_impl(ptr) const noexcept = 0;

        virtual const TreeIter _at_impl(const std::string&) const = 0;
        virtual TreeIter _create_impl(ptr) = 0;
        virtual TreeIter _override_impl(ptr) noexcept = 0;
        virtual void _remove_impl(const std::string&) = 0;
        virtual void _remove_impl(ptr) = 0;
        virtual void _clear_impl() noexcept = 0;
    public:
        using Element::Element;

        bool empty() const noexcept
        {
            return _empty_impl();
        }
        bool exists(const std::string& name) const noexcept
        {
            return _exists_impl(name);
        }
        bool exists(ptr ptr) const noexcept
        {
            return _exists_impl(ptr);
        }

        TreeIter at(const std::string& name) const
        {
            return _at_impl(name);
        }

        void layout_for(enum Layout layout, cptr elem)  const noexcept;

        int resolve_units(Unit, cptr) const noexcept;
        using Element::resolve_units;

        template<typename Type> TypedTreeIter<Type> create(const std::string& name = "")
        {
            return create(Element::make<Type>(
                              name,
                              state(),
                              std::static_pointer_cast<ParentElement>(shared_from_this())
                          ));
        }
        template<typename Type> TypedTreeIter<Type> override(const std::string& name = "")
        {
            return override(Element::make<Type>(
                                name,
                                state(),
                                std::static_pointer_cast<ParentElement>(shared_from_this())
                            ));
        }
        TreeIter create(ptr ptr)
        {
            ptr->setstate(State::Created);
            return _create_impl(ptr);
        }
        TreeIter override(ptr ptr) noexcept
        {
            ptr->setstate(State::Created);
            return _override_impl(ptr);
        }

        void clear() noexcept
        {
            _clear_impl();
        }

        void remove(const std::string& name)
        {
            _remove_impl(name);
        }
        void remove(ptr ptr)
        {
            _remove_impl(ptr);
        }

        int border_for(BorderType type, cptr ptr) const noexcept;

        virtual DynamicIterator begin() noexcept { return nullptr; }
        virtual DynamicIterator end() noexcept { return nullptr; }
        virtual void foreach_internal(foreach_internal_callback callback) const = 0;
        virtual void foreach(foreach_callback callback) const = 0;
    };
    class VecParentElement : public ParentElement
    {
    private:
        class LinearIteratorAdaptor;
    protected:
        std::vector<ptr> _elements{};

        virtual bool _empty_impl() const noexcept override;
        virtual bool _exists_impl(const std::string& name) const noexcept override;
        virtual bool _exists_impl(ptr ptr) const noexcept override;

        virtual const TreeIter _at_impl(const std::string& name) const override;
        virtual TreeIter _create_impl(ptr ptr) override;
        virtual TreeIter _override_impl(ptr ptr) noexcept override;
        virtual void _remove_impl(const std::string& name) override;
        virtual void _remove_impl(ptr ptr) override;
        virtual void _clear_impl() noexcept override;

        std::size_t _find(const std::string& name) const noexcept;
        std::size_t _find(ptr ptr) const noexcept;
    public:
        using ParentElement::ParentElement;

        virtual DynamicIterator begin() noexcept override;
        virtual DynamicIterator end() noexcept override;
        virtual void foreach_internal(foreach_internal_callback callback) const override;
        virtual void foreach(foreach_callback callback) const override;
    };
    class MetaParentElement : public ParentElement
    {
    protected:
        virtual bool _empty_impl() const noexcept override
        {
            return true;
        }
        virtual bool _exists_impl(const std::string& name) const noexcept override
        {
            return false;
        }
        virtual bool _exists_impl(ptr ptr) const noexcept override
        {
            return false;
        }

        virtual const TreeIter _at_impl(const std::string& name) const override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter _create_impl(ptr ptr) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter _override_impl(ptr ptr) noexcept override
        {
            return nullptr;
        }
        virtual void _remove_impl(const std::string& name) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual void _remove_impl(ptr ptr) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual void _clear_impl() noexcept override {}
    public:
        using ParentElement::ParentElement;

        virtual void foreach_internal(foreach_internal_callback callback) const override {}
        virtual void foreach (foreach_callback callback) const override {}
    };
}

#endif // D2_TREE_PARENT_HPP
