#ifndef D2_SCREEN_HPP
#define D2_SCREEN_HPP

#include "d2_io_handler.hpp"
#include "d2_interpolator.hpp"
#include "d2_model.hpp"
#include "d2_tree_parent.hpp"
#include <any>

namespace d2
{
    // Standard tags:
    // SwapClean - if on, the entire tree is deleted when out of view and rebuilt later on
    // SwapOut - if on, the tree is swapped out when out of view
    // OverrideRefresh - when the tree is set, refresh rate is overriden with the value
    class TreeTags
    {
    private:
        absl::flat_hash_map<std::string, std::any> _tags{};
    public:
        TreeTags()
        {
            set<bool>("SwapClean", false);
            set<bool>("SwapOut", true);
            set<std::chrono::milliseconds>("OverrideRefresh", std::chrono::milliseconds(0));
        }
        TreeTags(const TreeTags&) = default;
        TreeTags(TreeTags&&) = default;

        template<typename Type>
        void set(const std::string& name, Type&& value)
        {
            _tags[name] = std::forward<Type>(value);
        }

        template<typename Type>
        Type tag(const std::string& name) const
        {
            auto f = _tags.find(name);
            if (f == _tags.end())
                throw std::logic_error{ std::format("Attempt to access invalid tag: {}", name) };
            else if (f->second.type() != typeid(Type))
                throw std::logic_error{ std::format("Attempt to access tag through invalid type: {}", name) };
            return std::any_cast<Type>(f->second);
        }

        TreeTags& operator=(const TreeTags&) = default;
        TreeTags& operator=(TreeTags&&) = default;
    };
    class DynamicDependencyManager
    {
    private:
        absl::flat_hash_map<std::string, std::any> _deps{};
    public:
        template<typename Type>
        void set(const std::string& name, Type&& value)
        {
            using dtype = style::Theme::Dependency<std::remove_cvref_t<Type>>;
            auto& dep =_deps[name];
            dep.emplace<dtype>();
            std::any_cast<dtype&>(dep) = std::forward<Type>(value);
        }
        template<typename Type>
        style::Theme::Dependency<Type>& get(const std::string& name)
        {
            using dtype = style::Theme::Dependency<std::remove_cvref_t<Type>>;
            if (auto f = _deps.find(name);
                f == _deps.end())
            {
                set(name, Type());
                return std::any_cast<dtype&>(_deps[name]);
            }
            else
                return std::any_cast<dtype&>(f->second);
        }
    };

    class Screen : public std::enable_shared_from_this<Screen>
    {
    public:
        enum class Profile
        {
            Stable,
            Adaptive,
            Auto = Stable,
        };
        using ModelType = MatrixModel::ModelType;
        using ptr = std::shared_ptr<Screen>;
    private:
        struct TreeData
        {
            std::function<void(TypedTreeIter<ParentElement>, TreeState::ptr)> rebuild{ nullptr };
            std::list<interp::Interpolator::ptr> interpolators{};
            TreeState::ptr state{ nullptr };
            TreeTags tags{};
            DynamicDependencyManager deps{};
            bool unbuilt{ true };
            bool swapped_out{ true };
            bool constructed{ false };
        };
        using tree = std::shared_ptr<TreeData>;
        using eptr = TreeIter;
    private:
        IOContext::ptr _ctx{ nullptr };
        absl::flat_hash_map<std::string, tree> _trees{};
        absl::flat_hash_map<std::string, MatrixModel::ptr> _models{};
        absl::flat_hash_map<std::size_t, style::Theme::ptr> _themes{};
        std::string _current_name{ "" };

        ParentElement::DynamicIterator _keynav_iterator{ nullptr };
        eptr _focused{ nullptr };
        eptr _targetted{ nullptr };
        tree _current{ nullptr };

        std::size_t _fps_avg{ 0 };
        std::chrono::microseconds _prev_delta{};
        std::chrono::milliseconds _restore_refresh{};
        std::chrono::milliseconds _refresh_rate{ 0 };
        bool _is_suspended{ false };
        bool _is_stop{ false };
        bool _is_running{ false };

        void _keynav_cycle_up(eptr ptr);
        void _keynav_cycle();
        void _keynav_cycle_up_reverse();
        void _keynav_cycle_reverse();
        void _keynav_cycle_macro();
        void _keynav_cycle_macro_reverse();

        void _frame();

        void _run_interpolators();
        void _trigger_focused(IOContext::Event ev);
        void _trigger_hovered(IOContext::Event ev);
        void _trigger_events();
        void _trigger_focused_events();
        void _trigger_hovered_events();

        void _update_viewport();
        eptr _update_states(eptr container, const std::pair<int, int>& mouse);

        void _apply_impl(const Element::foreach_callback& func, eptr container) const;
    public:
        template<typename First, typename... Rest>
        static auto make(IOContext::ptr ctx, auto&&... themes)
        {
            auto ptr = std::make_shared<Screen>(ctx);
            ptr->make_theme(themes...);
            ptr->set<First, Rest...>();
            return ptr;
        }
        static auto make(IOContext::ptr ctx)
        {
            return std::make_shared<Screen>(ctx);
        }
        static constexpr std::chrono::milliseconds fps(std::size_t c)
        {
            if (!c) return std::chrono::milliseconds(0);
            return std::chrono::milliseconds(1000 / c);
        }

        explicit Screen(IOContext::ptr ctx) : _ctx(ctx) {}
        virtual ~Screen() = default;

        IOContext::ptr context() const;
        TypedTreeIter<ParentElement> root() const;

        // Model Management

        MatrixModel::ptr fetch_model(const std::string& name, const std::string& path);
        MatrixModel::ptr fetch_model(const std::string& name, const std::string& path, ModelType type);
        MatrixModel::ptr fetch_model(const std::string& name, int width, int height, std::vector<Pixel> data);
        MatrixModel::ptr fetch_model(const std::string& name, int width, int height, std::span<const Pixel> data);
        MatrixModel::ptr fetch_model(const std::string& name);
        void remove_model(const std::string& name);

        // Metadata (meta)

        void focus(eptr p);
        eptr focused() const;

        std::size_t fps() const;
        std::size_t animations() const;
        std::chrono::microseconds delta() const;

        bool is_keynav() const;

        // Access

        void apply(const Element::foreach_callback& func) const;
        void apply_all(const Element::foreach_callback& func) const;

        // Tree data

        // Loads the trees and sets the first in the list
        template<typename First, typename... Rest>
        void set()
        {
            auto load = [this]<typename Set>()
            {
                const std::string name = std::string(Set::name);
                auto& t = _trees[name];
                if (t == nullptr)
                {
                    t = std::make_shared<TreeData>();
                }
                else
                {
                    t->interpolators.clear();
                }
                t->state = Set::build(shared_from_this());
                t->rebuild = [](TypedTreeIter<ParentElement> root, TreeState::ptr state) {
                    Set::create_at(root, std::static_pointer_cast<typename Set::__state_type>(state));
                };
            };
            load.template operator()<First>();
            (load.template operator()<Rest>(), ...);
            set(std::string(First::name));
        }
        void set(const std::string& name);

        template<typename Type = void>
        auto state() const
        {
            if constexpr(std::is_same_v<Type, void>)
            {
                return TreeState::ptr(_current->state);
            }
            else
            {
                return std::static_pointer_cast<Type>(_current->state);
            }
        }
        template<typename Type = void>
        auto state(const std::string& name) const
        {
            auto f = _trees.find(name);
            if (f == _trees.end())
                throw std::logic_error{ "Invalid tree" };
            auto& state = *f->second->state;
            if (typeid(state) != typeid(Type))
                throw std::logic_error{ "State type mismatch" };

            if constexpr(std::is_same_v<Type, void>)
            {
                return TreeState::ptr(f->second->state);
            }
            else
            {
                return std::static_pointer_cast<Type>(f->second->state);
            }
        }

        TreeTags& tags();
        TreeTags& tags(const std::string& name);

        const TreeTags& tags() const;
        const TreeTags& tags(const std::string& name) const;

        DynamicDependencyManager& deps();
        DynamicDependencyManager& deps(const std::string& name);

        const DynamicDependencyManager& deps() const;
        const DynamicDependencyManager& deps(const std::string& name) const;

        // Interpolation

        template<typename Type, typename... Argv>
        interp::Interpolator::ptr interpolate(std::chrono::milliseconds time, Argv&&... args)
        {
            auto& interps = _current->interpolators;
            const auto ptr = std::make_shared<Type>(time, std::forward<Argv>(args)...);
            const auto code = ptr->hash_code();

            for (auto it = interps.begin(); it != interps.end();)
            {
                if ((*it)->hash_code() == code)
                {
                    const auto saved = it;
                    ++it;
                    interps.erase(saved);
                }
                else ++it;
            }

            interps.push_front(ptr);
            interps.front()->setowner(std::hash<tree>()(_current));
            interps.front()->start();
            return interps.front();
        }

        void clear_animations(Element::ptr ptr);

        void erase_tree(const std::string& name);
        void erase_tree();
        void clear_tree();

        // Theming

        template<typename Type>
        Type& theme()
        {
            const auto code = typeid(Type).hash_code();
            auto f = _themes.find(code);
            if (f == _themes.end())
            {
                for (decltype(auto) it : _themes)
                {
                    auto ptr = std::dynamic_pointer_cast<Type>(it.second);
                    if (ptr)
                    {
                        auto bptr = it.second;
                        _themes[code] = bptr;
                        return *ptr;
                    }
                }
                throw std::logic_error{ std::format("Failed to resolve theme dependency for: {}", typeid(Type).name()) };
            }
            return dynamic_cast<Type&>(*f->second);
        }

        template<typename Type>
        bool has_theme()
        {
            return _themes.contains(typeid(Type).hash_code());
        }

        template<typename... Themes>
        void make_theme(Themes&&... themes)
        {
            auto create = [this](auto theme)
            {
                const auto code = typeid(*theme).hash_code();
                if (_themes.contains(code))
                    throw std::logic_error{ "Double theme initialization (this could be caused by related themes)" };
                _themes[code] = theme;
            };
            ((create(themes)), ...);
        }

        // Rendering

        bool is_suspended() const;
        void suspend(bool state);

        void start_blocking(std::chrono::milliseconds refresh, Profile profile = Profile::Auto);
        void stop_blocking();
        void set_refresh_rate(std::chrono::milliseconds refresh);

        void render();
        void update();

        eptr traverse();
        eptr operator/(const std::string& path);
    };
}

#endif // D2_SCREEN_HPP
