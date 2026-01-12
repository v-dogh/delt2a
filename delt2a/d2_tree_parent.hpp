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

                void increment(int cnt, std::shared_ptr<ParentElement> elem)
                {
                    for (std::size_t i = 0; i < cnt; i++)
                        increment(elem);
                }
                void decrement(int cnt, std::shared_ptr<ParentElement> elem)
                {
                    for (std::size_t i = 0; i < cnt; i++)
                        decrement(elem);
                }

                virtual ptr value(std::shared_ptr<ParentElement> elem) const = 0;
                virtual void increment(std::shared_ptr<ParentElement> elem) = 0;
                virtual void decrement(std::shared_ptr<ParentElement> elem) = 0;
                virtual bool is_null(std::shared_ptr<ParentElement> elem) const = 0;
                virtual bool is_begin(std::shared_ptr<ParentElement> elem) const = 0;
                virtual bool is_end(std::shared_ptr<ParentElement> elem) const = 0;
                virtual bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const = 0;
                virtual std::unique_ptr<DynamicIteratorAdaptor> clone() const = 0;
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

            void increment(int cnt = 1);
            void decrement(int cnt = 1);

            bool is_begin() const;
            bool is_end() const;
            bool is_null() const;
            bool is_equal(DynamicIterator it) const;

            Element::ptr value() const;

            Element::ptr operator->() const;
            Element& operator*() const;

            bool operator==(const DynamicIterator& other) const;
            bool operator!=(const DynamicIterator& other) const;

            DynamicIterator& operator=(const DynamicIterator& copy);
            DynamicIterator& operator=(DynamicIterator&&) = default;
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

        virtual int _get_border_impl(BorderType, cptr) const { return 0; }
        virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) override;
        virtual void _state_change_impl(State state, bool value) override;

        virtual bool _empty_impl() const = 0;
        virtual bool _exists_impl(const std::string&) const = 0;
        virtual bool _exists_impl(ptr) const = 0;

        virtual TreeIter _at_impl(const std::string&) const = 0;
        virtual TreeIter _create_impl(ptr) = 0;
        virtual TreeIter _override_impl(ptr) = 0;
        virtual TreeIter _create_after_impl(ptr p, ptr after) = 0;
        virtual TreeIter _override_after_impl(ptr p, ptr after) = 0;
        virtual bool _remove_impl(const std::string&) = 0;
        virtual bool _remove_impl(ptr) = 0;
        virtual void _clear_impl() = 0;
    public:
        using Element::Element;

        bool empty() const;
        bool exists(const std::string& name) const;
        bool exists(ptr ptr) const;

        TreeIter at(const std::string& name) const;

        void layout_for(enum Layout layout, cptr elem)  const;

        int resolve_units(Unit, cptr) const;
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
        TreeIter create(ptr ptr);
        TreeIter override(ptr ptr);

        template<typename Type> TypedTreeIter<Type> create_after(ptr after, const std::string& name = "")
        {
            return create_after(Element::make<Type>(
                name,
                state(),
                std::static_pointer_cast<ParentElement>(shared_from_this())
            ), after);
        }
        template<typename Type> TypedTreeIter<Type> override_after(ptr after, const std::string& name = "")
        {
            return override_after(Element::make<Type>(
                name,
                state(),
                std::static_pointer_cast<ParentElement>(shared_from_this())
            ), after);
        }
        TreeIter create_after(ptr p, ptr after);
        TreeIter override_after(ptr p, ptr after);

        void clear();

        void remove(const std::string& name);
        void remove(ptr ptr);

        bool remove_if(const std::string& name);
        bool remove_if(ptr ptr);

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

        virtual TreeIter _at_impl(const std::string& name) const override;
        virtual TreeIter _create_impl(ptr ptr) override;
        virtual TreeIter _override_impl(ptr ptr) override;
        virtual TreeIter _create_after_impl(ptr p, ptr after) override;
        virtual TreeIter _override_after_impl(ptr p, ptr after) override;
        virtual bool _remove_impl(const std::string& name) override;
        virtual bool _remove_impl(ptr ptr) override;
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

        virtual TreeIter _at_impl(const std::string& name) const override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter _create_impl(ptr ptr) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter _override_impl(ptr ptr) override
        {
            return nullptr;
        }
        virtual TreeIter _create_after_impl(ptr p, ptr after) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual TreeIter _override_after_impl(ptr p, ptr after) override
        {
            return nullptr;
        }

        virtual bool _remove_impl(const std::string& name) override
        {
            throw std::logic_error{ "Not implemented" };
        }
        virtual bool _remove_impl(ptr ptr) override
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
