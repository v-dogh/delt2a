#ifndef D2_SCREEN_HPP
#define D2_SCREEN_HPP

#include "d2_io_handler.hpp"
#include "d2_interpolator.hpp"
#include "d2_model.hpp"
#include "d2_screen_comps.hpp"
#include "d2_tree_element_frwd.hpp"
#include "d2_tree_state.hpp"

namespace d2::sys
{
    class SystemScreen :
        public SystemComponentBase<"Screen", SystemScreen>,
        public SystemComponentCfg<false, false>
    {
        D2_TAG_MODULE(src)
    public:
        enum class Event
        {
            Resize,
            PreRedraw,
            PostRedraw,
            KeyInput,
            KeySequenceInput,
            MouseInput,
            Update,
        };
        using ModelType = MatrixModel::ModelType;
    private:
        struct TreeData
        {
            std::function<void(TreeIter<ParentElement>, TreeState::ptr)> rebuild{ nullptr };
            std::list<interp::Interpolator::ptr> interpolators{};
            TreeState::ptr state{ nullptr };
            TreeTags tags{};
            DynamicDependencyManager deps{};
            bool unbuilt{ true };
            bool swapped_out{ true };
        };
        using tree = std::shared_ptr<TreeData>;
        using eptr = TreeIter<>;
    private:
        Signals::Signal _sig{};

        absl::flat_hash_map<std::string, tree> _trees{};
        absl::flat_hash_map<std::string, MatrixModel::ptr> _models{};
        absl::flat_hash_map<std::size_t, style::Theme::ptr> _themes{};
        std::string _current_name{ "" };

        internal::DynamicIterator _keynav_iterator{ nullptr };
        eptr _focused{ nullptr };
        eptr _targetted{ nullptr };
        eptr _clicked{ nullptr };
        tree _current{ nullptr };

        std::size_t _fps_ctr{ 0 };
        std::size_t _fps_avg{ 0 };
        std::chrono::steady_clock::time_point _last_mes{};
        std::chrono::microseconds _prev_delta{};
        std::chrono::milliseconds _restore_refresh{};
        std::chrono::milliseconds _refresh_rate{ 0 };
        bool _is_stop{ false };
        bool _is_running{ false };

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;

        void _keynav_cycle_up(eptr ptr);
        void _keynav_cycle();
        void _keynav_cycle_up_reverse();
        void _keynav_cycle_reverse();
        void _keynav_cycle_macro();
        void _keynav_cycle_macro_reverse();

        void _run_interpolators();
        void _trigger_focused(Event ev);
        void _trigger_hovered(Event ev);
        void _trigger_events();
        void _trigger_focused_events();
        void _trigger_hovered_events();

        void _update_viewport();
        eptr _update_states(eptr container, const std::pair<int, int>& mouse);
        eptr _update_states_reverse(eptr ptr);

        void _apply_impl(const TreeIter<>::foreach_callback& func, eptr container) const;
        void _signal(Event ev);
    public:
        static constexpr std::chrono::milliseconds fps(std::size_t c)
        {
            if (!c) return std::chrono::milliseconds(0);
            return std::chrono::milliseconds(1000 / c);
        }

        using SystemComponentBase::SystemComponentBase;

        TreeIter<ParentElement> root() const;

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

        void apply(const TreeIter<>::foreach_callback& func) const;
        void apply_all(const TreeIter<>::foreach_callback& func) const;

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
                t->state = Set::build(context());
                t->rebuild = [](TreeIter<ParentElement> root, TreeState::ptr state) {
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

        void clear_animations(TreeIter<> ptr);

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
        Type& theme(const std::string& id)
        {
            const auto code = std::hash<std::string>()(id);
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
        bool has_theme(const std::string& id)
        {
            return _themes.contains(std::hash<std::string>()(id));
        }

        template<typename... Themes>
        void make_theme(Themes&&... themes)
        {
            auto create = [this](auto theme)
            {
                if (_themes.contains(theme->code()))
                    throw std::logic_error{ "Double theme initialization (this could be caused by related themes)" };
                _themes[theme->code()] = theme;
            };
            ((create(themes)), ...);
        }

        // Rendering

        void set_refresh_rate(std::chrono::milliseconds refresh);
        void tick();

        eptr traverse();
        eptr operator/(const std::string& path);
    };
}

namespace d2
{
    template<typename Tree, typename... Trees>
    void IOContext::run(auto&&... themes)
    {
        _initialize();
        auto src = screen();
        src->make_theme(themes...);
        src->set<Tree, Trees...>();
        _run();
        _deinitialize();
    }
}

#endif // D2_SCREEN_HPP
