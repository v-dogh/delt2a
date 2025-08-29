#ifndef D2_SCREEN_HPP
#define D2_SCREEN_HPP

#include "d2_io_handler.hpp"
#include "d2_interpolator.hpp"
#include "d2_model.hpp"
#include "d2_tree_parent.hpp"
#include <unordered_map>
#include <mutex>

namespace d2
{
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
            std::list<interp::Interpolator::ptr> interpolators{};
            TreeState::ptr state{ nullptr };
        };
        using tree = std::shared_ptr<TreeData>;
        using eptr = TreeIter;
    private:
        mutable std::recursive_mutex _mtx{};
        mutable std::mutex _models_mtx{};
        mutable std::mutex _interp_mtx{};

        IOContext::ptr _ctx{ nullptr };
        std::unordered_map<std::string, tree> _trees{};
        std::unordered_map<std::string, MatrixModel::ptr> _models{};
        std::unordered_map<std::size_t, style::Theme::ptr> _themes{};
        std::string _current_name{ "" };
        util::mt::AsynchronousScheduler::CyclicWorkerHandle _worker{};

        ParentElement::DynamicIterator _keynav_iterator{ nullptr };
        eptr _focused{ nullptr };
        eptr _targetted{ nullptr };
        tree _current{ nullptr };

        std::atomic<bool> _is_suspended{ false };
        std::atomic<bool> _is_stop{ false };
        std::atomic<bool> _is_running{ false };
        std::atomic<std::size_t> _fps_avg{ 0 };
        std::atomic<std::chrono::microseconds> _prev_delta{};
        std::atomic<std::chrono::milliseconds> _refresh_rate{};
        std::atomic<std::thread::id> _main_thread_id{};

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
        static std::chrono::milliseconds fps(std::size_t c)
        {
            if (!c) return std::chrono::milliseconds(0);
            return std::chrono::milliseconds(1000 / c);
        }

        explicit Screen(IOContext::ptr ctx) : _ctx(ctx) {}
        virtual ~Screen() = default;

        IOContext::ptr context() const noexcept;
        eptr root() const noexcept;

        // Model Management

        MatrixModel::ptr fetch_model(const std::string& name, const std::string& path);
        MatrixModel::ptr fetch_model(const std::string& name, const std::string& path, ModelType type);
        MatrixModel::ptr fetch_model(const std::string& name, int width, int height, std::vector<Pixel> data);
        MatrixModel::ptr fetch_model(const std::string& name, int width, int height, std::span<const Pixel> data);
        MatrixModel::ptr fetch_model(const std::string& name);
        void remove_model(const std::string& name);

        // Synchronization

        bool is_synced() const noexcept
        {
            return (std::this_thread::get_id() == _main_thread_id) || !_is_running;
        }
        template<typename Func>
        auto sync(Func&& callback) -> util::mt::future<decltype(callback())>
        {
            return _ctx->scheduler()->launch_deferred_task([callback = std::forward<Func>(callback)]() -> decltype(auto)
            {
                return callback();
            }, std::chrono::milliseconds(0));
        }
        template<typename Func>
        auto sync_if(Func&& callback) -> decltype(callback())
        {
            if (is_synced())
            {
                return callback();
            }
            else
            {
                return _ctx->scheduler()->launch_deferred_task([callback = std::forward<Func>(callback)]() -> decltype(auto)
                {
                    return callback();
                }, std::chrono::milliseconds(0)).value();
            }
        }

        // Metadata (meta)

        void focus(eptr p);
        eptr focused() const noexcept;

        std::size_t fps() const noexcept;
        std::size_t animations() const noexcept;
        std::chrono::microseconds delta() const noexcept;

        bool is_keynav() const noexcept;

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
                std::unique_lock lock(_mtx);
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

                // set(name) swaps out the current if it's not nullptr
                // However we still need it for the tree in case any callbacks need to access the current tree state

                _current = t;
                _current_name = Set::name;

                Set::create_at(t->state->root(), t->state);

                _current_name = "";
                _current = nullptr;
            };
            load.template operator()<First>();
            (load.template operator()<Rest>(), ...);
            set(std::string(First::name));
        }
        void set(const std::string& name);

        template<typename Type = void>
        auto getstate() const
        {
            if constexpr(std::is_same_v<Type, void>)
            {
                return TreeState::ptr(_current->state);
            }
            else
            {
                for (decltype(auto) it : _trees)
                {
                    auto& state = *it.second->state;
                    if (typeid(state) == typeid(Type))
                        return std::static_pointer_cast<Type>(it.second->state);
                }
                throw std::logic_error{ "Invalid state" };
            }
        }

        // Interpolation

        template<typename Type, typename... Argv>
        interp::Interpolator::ptr interpolate(std::chrono::milliseconds time, Argv&&... args) noexcept
        {
            std::unique_lock lock(_interp_mtx);
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

        void clear_animations(Element::ptr ptr) noexcept;

        void erase_tree(const std::string& name);
        void erase_tree();
        void clear_tree();

        // Theming

        template<typename Type>
        Type& theme()
        {
            std::unique_lock lock(_mtx);
            const auto code = typeid(Type).hash_code();
            auto f = _themes.find(code);
            if (f == _themes.end())
            {
                for (decltype(auto) it : _themes)
                {
                    auto ptr = std::dynamic_pointer_cast<Type>(it.second);
                    if (ptr)
                    {
                        _themes[code] = it.second;
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
            std::unique_lock lock(_mtx);
            return _themes.contains(typeid(Type).hash_code());
        }

        template<typename... Themes>
        void make_theme(Themes&&... themes)
        {
            auto create = [this](auto theme)
            {
                const auto code = typeid(*theme).hash_code();
                if (_themes.contains(code))
                    return;
                _themes[code] = theme;
            };

            std::unique_lock lock(_mtx);
            ((create(themes)), ...);
        }

        // Rendering

        bool is_suspended() const noexcept;
        void suspend(bool state) noexcept;

        void start_blocking(std::chrono::milliseconds refresh, Profile profile = Profile::Auto);
        void stop_blocking() noexcept;
        void set_refresh_rate(std::chrono::milliseconds refresh) noexcept;

        void render();
        void update();

        eptr traverse();
        eptr operator/(const std::string& path);
    };
}

#endif // D2_SCREEN_HPP
