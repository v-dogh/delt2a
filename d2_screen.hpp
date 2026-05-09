#ifndef D2_SCREEN_HPP
#define D2_SCREEN_HPP

#include <chrono>
#include <d2_input_base.hpp>
#include <d2_interpolator.hpp>
#include <d2_io_handler.hpp>
#include <d2_model.hpp>
#include <d2_module.hpp>
#include <d2_screen_comps.hpp>
#include <d2_theme.hpp>
#include <d2_tree_element_frwd.hpp>
#include <d2_tree_state.hpp>
#include <mods/d2_core.hpp>

namespace d2::sys
{
    class SystemScreen : public AbstractModule<"Screen">,
                         public ConcreteModule<
                             SystemScreen,
                             Access::TUnsafe,
                             Load::Immediate,
                             SystemInput,
                             SystemOutput
                         >
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
            KeyMouseInput,
            MouseMoved,
            MouseInput,
            Update,
        };
        using ModelType = MatrixModel::ModelType;
    private:
        struct TreeData
        {
            std::function<void(TreeIter<ParentElement>, TreeState::ptr)> rebuild{nullptr};
            std::list<interp::Interpolator::ptr> interpolators{};
            TreeState::ptr state{nullptr};
            TreeTags tags{};
            DynamicDependencyManager deps{};
            bool unbuilt{true};
            bool swapped_out{true};
        };
        using tree = std::shared_ptr<TreeData>;
        using eptr = TreeIter<>;
        struct TempTreeState
        {
            internal::DynamicIterator keynav_iterator{nullptr};
            std::string current_name{""};
            eptr focused{nullptr};
            eptr targetted{nullptr};
            eptr clicked{nullptr};
            tree current{nullptr};
            bool recursive{false};
        };
    private:
        Signals::Signal _sig{};

        absl::flat_hash_map<std::string, tree> _trees{};
        absl::flat_hash_map<std::string, MatrixModel::ptr> _models{};
        absl::flat_hash_map<std::size_t, style::Theme::ptr> _themes{};

        in::InputFrame* _frame{nullptr};
        TempTreeState _ts{};

        std::size_t _fps_ctr{0};
        std::size_t _fps_avg{0};
        std::chrono::steady_clock::time_point _last_mes{};
        std::chrono::microseconds _prev_delta{};
        std::chrono::milliseconds _restore_refresh{};
        std::chrono::milliseconds _refresh_rate{std::chrono::milliseconds::max()};
        bool _is_stop{false};
        bool _is_running{false};

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;

        eptr _lca(eptr a, eptr b);

        void _keynav_cycle_up(eptr ptr);
        void _keynav_cycle();
        void _keynav_cycle_up_reverse();
        void _keynav_cycle_reverse();
        void _keynav_cycle_macro();
        void _keynav_cycle_macro_reverse();

        void _run_interpolators();
        void _trigger_events();
        void _trigger_focused_events(in::InputFrame& frame, eptr ptr);
        void _trigger_focused_events(in::InputFrame& frame);
        void _trigger_hovered_events(in::InputFrame& frame, eptr ptr);
        void _trigger_hovered_events(in::InputFrame& frame);
        void _trigger_rc_hover_events(eptr n, eptr o);
        void _trigger_rc_click_events(eptr n, eptr o);
        void _trigger_rc_focus_events(eptr n, eptr o);

        void _update_viewport();
        eptr _update_states(eptr container, const std::pair<int, int>& mouse);
        eptr _update_states_reverse(eptr ptr);

        void _apply_impl(const TreeIter<>::foreach_callback& func, eptr container) const;
        void _signal(Event ev);
    public:
        static constexpr std::chrono::milliseconds fps(std::size_t c)
        {
            if (!c)
                return std::chrono::milliseconds(0);
            return std::chrono::milliseconds(1000 / c);
        }

        using AbstractModule::AbstractModule;

        TreeIter<ParentElement> root() const;

        // Model Management

        MatrixModel::ptr fetch_model(const std::string& name, const std::string& path);
        MatrixModel::ptr
        fetch_model(const std::string& name, const std::string& path, ModelType type);
        MatrixModel::ptr
        fetch_model(const std::string& name, int width, int height, std::vector<Pixel> data);
        MatrixModel::ptr
        fetch_model(const std::string& name, int width, int height, std::span<const Pixel> data);
        MatrixModel::ptr fetch_model(const std::string& name);
        void remove_model(const std::string& name);

        // Metadata (meta)

        void focus(eptr p);
        eptr focused() const;
        eptr clicked() const;
        eptr hovered() const;

        std::size_t fps() const;
        std::size_t animations() const;
        std::chrono::microseconds delta() const;

        bool is_keynav() const;

        // Access

        void apply(const TreeIter<>::foreach_callback& func) const;
        void apply_all(const TreeIter<>::foreach_callback& func) const;

        // Tree data

        // Loads the trees and sets the first in the list
        template<typename First, typename... Rest> void set()
        {
            if (_ts.recursive)
                D2_THRW("Attempt to create trees recursively");
            _ts.recursive = true;
            auto load = [this]<typename Set>()
            {
                const std::string name = std::string(Set::name);
                auto& t = _trees[name];
                t = std::make_shared<TreeData>();
                // Temporary switch for Set::construct
                {
                    auto backup = std::move(_ts);
                    _ts = TempTreeState();
                    _ts.current_name = name;
                    _ts.current = t;
                    t->state = Set::build(context());
                    _ts = std::move(backup);
                }
                t->rebuild = [](TreeIter<ParentElement> root, TreeState::ptr state)
                {
                    Set::create_at(
                        root, std::static_pointer_cast<typename Set::__state_type>(state)
                    );
                };
            };
            load.template operator()<First>();
            (load.template operator()<Rest>(), ...);
            _ts.recursive = false;
            set(std::string(First::name));
        }
        void set(const std::string& name);

        template<typename Type = void> auto state() const
        {
            if constexpr (std::is_same_v<Type, void>)
            {
                return TreeState::ptr(_ts.current->state);
            }
            else
            {
                return std::static_pointer_cast<Type>(_ts.current->state);
            }
        }
        template<typename Type = void> auto state(const std::string& name) const
        {
            auto f = _trees.find(name);
            if (f == _trees.end())
                throw std::logic_error{"Invalid tree"};
            auto& state = *f->second->state;
            if (typeid(state) != typeid(Type))
                throw std::logic_error{"State type mismatch"};

            if constexpr (std::is_same_v<Type, void>)
            {
                return TreeState::ptr(f->second->state);
            }
            else
            {
                return std::static_pointer_cast<Type>(f->second->state);
            }
        }

        // Returns the current input frame for the tick
        in::InputFrame* input();

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
            auto& interps = _ts.current->interpolators;
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
                else
                    ++it;
            }

            interps.push_front(ptr);
            interps.front()->setowner(std::hash<tree>()(_ts.current));
            interps.front()->start();
            return interps.front();
        }

        void clear_animations(TreeIter<> ptr);

        void erase_tree(const std::string& name);
        void erase_tree();
        void clear_tree();

        // Theming

        template<typename Type> Type& theme()
        {
            auto f = _themes.find(Type::tc);
            if (f == _themes.end())
            {
                for (decltype(auto) it : _themes)
                {
                    auto ptr = std::dynamic_pointer_cast<Type>(it.second);
                    if (ptr)
                    {
                        auto bptr = it.second;
                        _themes[Type::tc] = bptr;
                        return *ptr;
                    }
                }
                throw std::logic_error{
                    std::format("Failed to resolve theme dependency for: {}", typeid(Type).name())
                };
            }
            return dynamic_cast<Type&>(*f->second);
        }
        template<typename Type> Type& theme(const std::string& id)
        {
            const auto code = style::Theme::id_to_code(id);
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
                throw std::logic_error{
                    std::format("Failed to resolve theme dependency for: {}", typeid(Type).name())
                };
            }
            return dynamic_cast<Type&>(*f->second);
        }

        template<auto Var> auto& themev()
        {
            return theme<typename style::ThemeRegistry<decltype(Var)>::type>().template get<Var>();
        }
        template<auto Var> auto& themev(const std::string& id)
        {
            return theme<typename style::ThemeRegistry<decltype(Var)>::type>().template get<Var>();
        }

        template<typename Type> bool has_theme()
        {
            return _themes.contains(typeid(Type).hash_code());
        }
        bool has_theme(const std::string& id)
        {
            return _themes.contains(std::hash<std::string>()(id));
        }

        template<typename... Themes> void make_theme(Themes&&... themes)
        {
            auto create = [this](auto theme)
            {
                if (_themes.contains(theme->code()))
                    throw std::logic_error{
                        "Double theme initialization (this could be caused by related themes)"
                    };
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
} // namespace d2::sys

namespace d2
{
    template<typename Tree, typename... Trees> void IOContext::run(auto&&... themes)
    {
        _initialize();
        auto src = screen();
        src->make_theme(themes...);
        src->set<Tree, Trees...>();
        _run();
        _deinitialize();
    }
} // namespace d2

#endif // D2_SCREEN_HPP
