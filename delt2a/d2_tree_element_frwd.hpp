#ifndef D2_TREE_ELEMENT_FRWD_HPP
#define D2_TREE_ELEMENT_FRWD_HPP

#include <functional>
#include <memory>
#include "d2_exceptions.hpp"

namespace d2
{
	using element_write_flag = unsigned char;
	class Element;
	class ParentElement;

    template<typename Type = Element>
    class TreeIter
    {
        D2_TAG_MODULE(tree)
    public:
        using type = Type;
        using foreach_callback = std::function<bool(TreeIter<>)>;
    private:
        std::weak_ptr<Type> _ptr{};
    public:
        TreeIter() = default;
        TreeIter(std::nullptr_t) {}
        TreeIter(const TreeIter&) = default;
        TreeIter(TreeIter&&) = default;
        TreeIter(std::weak_ptr<Type> ptr) : _ptr(ptr) {}
        template<typename Other> TreeIter(TreeIter<Other> ptr) : TreeIter(ptr.weak()) {}
        template<typename Other> TreeIter(std::shared_ptr<Other> ptr) : TreeIter(std::weak_ptr<Other>(ptr)) {}
        template<typename Other> TreeIter(std::weak_ptr<Other> ptr)
        {
            if (!ptr.expired())
            {
                auto p = std::dynamic_pointer_cast<Type>(ptr.lock());
                if (p == nullptr)
                    D2_THRW("Attempt to access an element through an invalid object");
                _ptr = p;
            }
        }

        std::shared_ptr<Type> shared() const { return _ptr.lock(); }
        std::weak_ptr<Type> weak() const { return _ptr; }

        template<typename Other>
        bool is_type() const
        {
            if (_ptr.expired())
                return false;
            return dynamic_cast<const Other*>(_ptr.lock().get()) != nullptr;
        }
        bool is_type_of(TreeIter<> other) const
        {
            if (_ptr.expired())
                return false;
            return typeid(other.shared().get()) == typeid(_ptr.lock().get());
        }

        template<typename Other = Element> TreeIter<Other> as() const
        {
            auto p = std::dynamic_pointer_cast<Other>(_ptr.lock());
            if (p == nullptr)
                D2_THRW("Attempt to access an element through an invalid object");
            return p;
        }
        TreeIter<ParentElement> asp() const { return as<ParentElement>(); }

        TreeIter<ParentElement> up() const
        {
            return shared()->parent();
        }
        TreeIter<ParentElement> up(std::size_t cnt) const
        {
            if (!cnt)
                return _ptr;
            return up().up(cnt - 1);
        }
        TreeIter<ParentElement> up(const std::string& name) const
        {
            auto current = _ptr.lock();
            while (current->name() != name)
            {
                current = current->parent();
                if (current == nullptr)
                    D2_THRW(std::format("Failed to locate parent with ID: {}", name));
            }
            return current;
        }

        void foreach(foreach_callback callback) const { asp()->foreach(callback); }

        bool operator==(const TreeIter& other) const
        {
            return (_ptr.expired() && other._ptr.expired()) ||
                   (_ptr.lock() == other._ptr.lock());
        }
        bool operator!=(const TreeIter& other) const {  return !operator==(other); }

        std::shared_ptr<Type> operator->() const { return shared(); }
        Type& operator*() const { return *shared(); }

        TreeIter<> operator/(const std::string& path) const { return asp()->at(path); }
        TreeIter<ParentElement> operator^(std::size_t cnt) const { return up(cnt); }
        TreeIter<ParentElement> operator^(const std::string& name) const { return up(name); }

        operator std::shared_ptr<Element>() const { return shared(); }
        operator std::weak_ptr<Element>() const { return weak(); }

        operator std::shared_ptr<Type>() const requires (!std::is_same_v<Type, Element>) { return shared(); }
        operator std::weak_ptr<Type>() const requires (!std::is_same_v<Type, Element>) { return weak(); }

        TreeIter& operator=(const TreeIter&) = default;
        TreeIter& operator=(TreeIter&&) = default;
    };

    namespace internal
    {
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

                virtual TreeIter<> value(std::shared_ptr<ParentElement> elem) const = 0;
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

            TreeIter<> value() const;

            TreeIter<> operator->() const;
            Element& operator*() const;

            bool operator==(const DynamicIterator& other) const;
            bool operator!=(const DynamicIterator& other) const;

            DynamicIterator& operator=(const DynamicIterator& copy);
            DynamicIterator& operator=(DynamicIterator&&) = default;
        };
    }
}

#endif // D2_TREE_ELEMENT_FRWD_HPP
