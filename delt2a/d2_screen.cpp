#include "d2_screen.hpp"
#include "elements/d2_box.hpp"
#include "d2_exceptions.hpp"
#include <filesystem>

namespace d2
{
    TreeState::TreeState(
        std::shared_ptr<Screen> src,
        std::shared_ptr<ParentElement> rptr,
        std::shared_ptr<ParentElement> coreptr
    ) : src_(src), root_ptr_(rptr), core_ptr_(coreptr)
    {}

    Screen::Screen(IOContext::ptr ctx)
        : _ctx(ctx)
    {
        _ctx->connect<Event, Screen::ptr>();
    }

    void TreeState::set_root(std::shared_ptr<ParentElement> ptr)
    {
        root_ptr_ = ptr;
    }
    void TreeState::set_core(std::shared_ptr<ParentElement> ptr)
    {
        core_ptr_ = ptr;
    }

    std::shared_ptr<IOContext> TreeState::context() const
    {
        return src_->context();
    }
    std::shared_ptr<Screen> TreeState::screen() const
    {
        return src_;
    }
    std::shared_ptr<ParentElement> TreeState::root() const
    {
        return root_ptr_;
    }
    std::shared_ptr<TreeState> TreeState::root_state() const
    {
        return root()->state();
    }
    std::shared_ptr<ParentElement> TreeState::core() const
    {
        return core_ptr_.lock();
    }

    void Screen::_keynav_cycle_up(eptr ptr)
    {
        const auto cparent = ptr->parent();
        const auto parent = cparent->parent();

        if (parent == nullptr)
        {
            _keynav_iterator = root().asp()->begin();
        }
        else
        {
            _keynav_iterator = parent->traverse().asp()->begin();
            while (_keynav_iterator->traverse() != cparent)
                _keynav_iterator.increment();

            if (_keynav_iterator.is_end())
            {
                _keynav_cycle_up(cparent);
            }
            else
            {
                _keynav_iterator.increment();
            }
            if (_keynav_iterator.is_end())
            {
                _keynav_cycle_up(cparent);
            }
        }
    }
    void Screen::_keynav_cycle()
    {
        if (_keynav_iterator == nullptr)
        {
            _keynav_iterator = root().asp()->begin();
            if (_keynav_iterator->provides_input())
                return;
        }

        do
        {
            if (_keynav_iterator->traverse().is_type<ParentElement>() &&
                    !_keynav_iterator->traverse().asp()->empty())
            {
                _keynav_iterator =
                    _keynav_iterator->traverse().asp()->begin();
            }
            else
            {
                eptr ptr = _keynav_iterator->traverse();
                _keynav_iterator.increment();
                if (_keynav_iterator.is_end())
                {
                    _keynav_cycle_up(ptr);
                }
            }
        }
        while (!_keynav_iterator->provides_input());
    }
    void Screen::_keynav_cycle_up_reverse()
    {
        const auto cparent = _keynav_iterator->parent();
        const auto parent = cparent->parent();

        if (parent == nullptr)
        {
            _keynav_iterator = root().asp()->end();
            _keynav_iterator.decrement();
        }
        else
        {
            _keynav_iterator = parent->traverse().asp()->begin();
            while (_keynav_iterator->traverse() != cparent)
                _keynav_iterator.increment();

            if (_keynav_iterator.is_begin())
            {
                _keynav_cycle_up_reverse();
            }
            else
            {
                _keynav_iterator.decrement();
            }
        }
    }
    void Screen::_keynav_cycle_reverse()
    {
        if (_keynav_iterator == nullptr)
        {
            _keynav_iterator = root().asp()->end();
            _keynav_iterator.decrement();
            if (_keynav_iterator->provides_input())
                return;
        }

        do
        {
            if (_keynav_iterator->traverse().is_type<ParentElement>() &&
                    !_keynav_iterator->traverse().asp()->empty())
            {
                _keynav_iterator =
                    _keynav_iterator->traverse().asp()->end();
                _keynav_iterator.decrement();
            }
            else
            {
                if (_keynav_iterator.is_begin())
                {
                    _keynav_cycle_up_reverse();
                }
                else
                {
                    _keynav_iterator.decrement();
                }
            }
        }
        while (!_keynav_iterator->provides_input());
    }

    void Screen::_keynav_cycle_macro()
    {
        if (_keynav_iterator == nullptr)
        {
            _keynav_iterator = root().asp()->begin();
            if (_keynav_iterator->provides_input())
                return;
        }

        auto parent = _keynav_iterator->parent();
        do _keynav_cycle();
        while (_keynav_iterator->parent() == parent);
    }
    void Screen::_keynav_cycle_macro_reverse()
    {
        if (_keynav_iterator == nullptr)
        {
            _keynav_iterator = root().asp()->end();
            _keynav_iterator.decrement();
            if (_keynav_iterator->provides_input())
                return;
        }

        auto parent = _keynav_iterator->parent();
        do _keynav_cycle_reverse();
        while (_keynav_iterator->parent() == parent);
    }

    void Screen::_frame()
    {
        _ctx->input()->endcycle();
        update();
        _ctx->input()->begincycle();
        render();
    }
    void Screen::_run_interpolators()
    {
        auto& interps = _current->interpolators;
        if (!interps.empty())
        {
            const auto end = interps.end();
            const auto beg = interps.begin();
            for (auto it = beg; it != end;)
            {
                if ((*it)->getowner() == std::hash<tree>()(_current))
                {
                    if (!(*it)->keep_alive())
                    {
                        const auto saved = it;
                        ++it;
                        interps.erase(saved);
                        if (it == interps.end()) break;
                    }
                    else
                    {
                        (*it)->update();
                        ++it;
                    }
                }
            }
        }
    }
    void Screen::_trigger_focused(Event ev)
    {
        internal::ElementView::from(_focused).trigger_event(ev);
    }
    void Screen::_trigger_hovered(Event ev)
    {
        internal::ElementView::from(_targetted).trigger_event(ev);
    }
    void Screen::_trigger_events()
    {
        auto* input = _ctx->input();
        const bool is_mouse_input = input->is_mouse_input();
        const bool is_keyboard_input = input->is_key_input();
        const bool is_key_seq_input = input->is_key_sequence_input();
        const bool is_resize = input->is_screen_resize();

        _signal(Event::Update);
        if (is_mouse_input) _signal(Event::MouseInput);
        if (is_resize) _signal(Event::Resize);
        if (is_keyboard_input) _signal(Event::KeyInput);
        if (is_key_seq_input) _signal(Event::KeySequenceInput);

        _trigger_focused_events();
        _trigger_hovered_events();
    }
    void Screen::_trigger_focused_events()
    {
        auto* input = _ctx->input();
        const auto is_mouse_input = input->is_mouse_input();
        const auto is_keyboard_input = input->is_key_input();
        const auto is_key_seq_input = input->is_key_sequence_input();
        const auto is_resize = input->is_screen_resize();

        if (_focused != nullptr)
        {
            _trigger_focused(Event::Update);
            if (is_mouse_input) _trigger_focused(Event::MouseInput);
            if (is_resize) _trigger_focused(Event::Resize);
            if (is_keyboard_input) _trigger_focused(Event::KeyInput);
            if (is_key_seq_input) _trigger_focused(Event::KeySequenceInput);
        }
    }
    void Screen::_trigger_hovered_events()
    {
        auto* input = _ctx->input();
        const auto is_mouse_input = input->is_mouse_input();

        if (_targetted != nullptr && _targetted != _focused)
        {
            _trigger_hovered(Event::Update);
            if (is_mouse_input) _trigger_hovered(Event::MouseInput);
        }
    }
    void Screen::_update_viewport()
    {
        const auto b = root().as<dx::Box>();
        b->accept_layout();
        const auto [ width, height ] = _ctx->input()->screen_size();
        const auto [ pwidth, pheight ] = b->box();
        const auto is_width = pwidth != width;
        const auto is_height = pheight != height;
        if (is_width || is_height)
        {
            auto i = internal::ElementView::from(b);
            b->override_dimensions(width, height);
            i.signal_write(Element::WriteType::Style);
            i.signal_context_change(
                (Element::WriteType::LayoutWidth * is_width) |
                (Element::WriteType::LayoutHeight * is_height)
            );
        }
    }
    Screen::eptr Screen::_update_states(eptr container, const std::pair<int, int>& mouse)
    {
        if (!container.is_type<ParentElement>())
            return nullptr;

        eptr mouse_target{ nullptr };
        // Press B to pass the vibe check
        container.asp()->foreach_internal([&](eptr it) -> bool
        {
            if (it->getstate(Element::State::Display))
            {
                const auto [ width, height ] = it->box();
                const auto [ x, y ] = it->position();
                if (
                    mouse_target == nullptr ||
                    it->getzindex() > mouse_target->getzindex()
                )
                {
                    if (
                        mouse.first >= x && mouse.second >= y &&
                        mouse.first < (x + width) && mouse.second < (y + height)
                    )
                    {
                        mouse_target = it;
                        auto res = _update_states(it, std::pair(mouse.first - x, mouse.second - y));
                        if (res != nullptr && res->getzindex() > dx::Box::underlap)
                            mouse_target = std::move(res);
                    }
                }
            }
            return true;
        });
        return mouse_target;
    }
    Screen::eptr Screen::_update_states_reverse(eptr ptr)
    {
        if (ptr->provides_cursor_sink())
            return ptr;
        if (ptr->parent() != nullptr)
            return _update_states_reverse(ptr->parent());
        return nullptr;
    }

    void Screen::_apply_impl(const Element::foreach_callback& func, eptr container) const
    {
        container.foreach([&func, this](eptr it)
        {
            func(it);
            if (it.is_type<ParentElement>())
                _apply_impl(func, it);
            return true;
        });
    }
    void Screen::_signal(Event ev)
    {
        _ctx->signal(ev, shared_from_this());
    }

    MatrixModel::ptr Screen::fetch_model(const std::string& name, const std::string& path)
    {
        const auto e = std::filesystem::path(name).extension();
        ModelType type = ModelType::Raw;
        if (e == "d2vm" || e == "ascii" || e == "txt") type = ModelType::Visual;
        else if (e == "d2m") type = ModelType::Raw;
        else throw std::runtime_error{ "Invalid model type" };
        return fetch_model(name, path, type);
    }
    MatrixModel::ptr Screen::fetch_model(const std::string& name, const std::string& path, ModelType type)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->load_model(path, type);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr Screen::fetch_model(const std::string& name, int width, int height, std::vector<Pixel> data)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset(std::move(data), width, height);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr Screen::fetch_model(const std::string& name, int width, int height, std::span<const Pixel> data)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset({ data.begin(), data.end() }, width, height);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr Screen::fetch_model(const std::string& name)
    {
        return _models[name];
    }
    void Screen::remove_model(const std::string& name)
    {
        _models.erase(name);
    }

    IOContext::ptr Screen::context() const
    {
        return _ctx;
    }
    TypedTreeIter<ParentElement> Screen::root() const
    {
        if (_current == nullptr)
            return nullptr;
        return _current->state->root();
    }

    void Screen::set(const std::string& name)
    {
        if (_current != nullptr && name == root()->name())
            return;

        auto root = _trees[name];
        if (_current != nullptr)
        {
            if (_current->tags.tag<bool>("SwapOut"))
            {
                _current->state->swap_out();
                this->root()->setstate(Element::Swapped, true);
                _current->swapped_out = true;
            }
            if (_current->tags.tag<bool>("SwapClean"))
            {
                _current->state->root()->clear();
                _current->unbuilt = true;
            }
            if (const auto value = _current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
                value != std::chrono::milliseconds(0))
            {
                set_refresh_rate(_restore_refresh);
            }
        }
        _current_name = name;
        _current = root;
        _focused = nullptr;

        if (_current->unbuilt)
        {
            _current->rebuild(
                this->root(), _current->state
            );
            _current->unbuilt = false;
        }
        if (_current->swapped_out)
        {
            root->state->swap_in();
            this->root()->setstate(Element::Swapped, false);
            _current->swapped_out = false;
        }

        if (const auto value = _current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
            value != std::chrono::milliseconds(0))
        {
            _restore_refresh = _refresh_rate;
            set_refresh_rate(value);
        }

        _ctx->input()->endcycle();
        _ctx->input()->begincycle();
        _update_viewport();

        root->state->root()->initialize(true);
    }

    TreeTags& Screen::tags()
    {
        return _current->tags;
    }
    TreeTags& Screen::tags(const std::string& name)
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            throw std::logic_error{ "Invalid tree" };
        return f->second->tags;
    }

    const TreeTags& Screen::tags() const
    {
        return _current->tags;
    }
    const TreeTags& Screen::tags(const std::string& name) const
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            throw std::logic_error{ "Invalid tree" };
        return f->second->tags;
    }

    DynamicDependencyManager& Screen::deps()
    {
        return _current->deps;
    }
    DynamicDependencyManager& Screen::deps(const std::string& name)
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            throw std::logic_error{ "Invalid tree" };
        return f->second->deps;
    }

    const DynamicDependencyManager& Screen::deps() const
    {
        return _current->deps;
    }
    const DynamicDependencyManager& Screen::deps(const std::string& name) const
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            throw std::logic_error{ "Invalid tree" };
        return f->second->deps;
    }

    void Screen::focus(eptr p)
    {
        if (p != _focused)
        {
            if (_focused != nullptr)
            {
                auto _ = _focused.shared();
                _focused->setstate(Element::Focused, false);
                _focused->setstate(Element::State::Clicked, false);
                _focused->setstate(Element::State::Hovered, false);
            }
            {
                _focused = p;
                if (p != nullptr)
                    _focused->setstate(Element::Focused);
            }
        }
    }
    Screen::eptr Screen::focused() const
    {
        return _focused;
    }

    std::size_t Screen::fps() const
    {
        return _fps_avg;
    }
    std::size_t Screen::animations() const
    {
        if (_current == nullptr)
            return 0;
        return _current->interpolators.size();
    }
    std::chrono::microseconds Screen::delta() const
    {
        return _prev_delta;
    }

    bool Screen::is_keynav() const
    {
        return
            _keynav_iterator != nullptr &&
            _focused == _keynav_iterator.value();
    }

    void Screen::apply(const Element::foreach_callback& func) const
    {
        _apply_impl(func, root());
    }
    void Screen::apply_all(const Element::foreach_callback& func) const
    {
        for (decltype(auto) it : _trees)
            _apply_impl(func, it.second->state->root());
    }

    void Screen::clear_animations(Element::ptr ptr)
    {
        auto& interps = _current->interpolators;
        for (auto it = interps.begin(); it != interps.end();)
        {
            if ((*it)->target() == ptr.get())
            {
                const auto saved = it;
                ++it;
                interps.erase(saved);
            }
            else ++it;
        }
    }

    void Screen::erase_tree(const std::string& name)
    {
        auto root = _trees[name];
        if (root == _current)
        {
            _focused = nullptr;
            _current = nullptr;
            _current_name = "";
        }
        _trees.erase(name);
    }
    void Screen::erase_tree()
    {
        erase_tree(_current_name);
    }
    void Screen::clear_tree()
    {
        _trees.clear();
        _current = nullptr;
        _focused = nullptr;
        _current_name = "";
    }

    bool Screen::is_suspended() const
    {
        return _is_suspended;
    }
    void Screen::suspend(bool state)
    {
        _is_suspended = state;
    }

    void Screen::start_blocking(std::chrono::milliseconds refresh_min, Profile profile)
    {
        ExtendedCodePage::activate_thread();

        // Adaptive FPS constants

        constexpr auto growth_factor = 1.5;
        constexpr auto shrink_factor = 0.7;
        constexpr auto decay_per_second = 20.0;

        double activity = 0.0;
        const auto in = context()->input();
        auto prev_measurement = std::chrono::steady_clock::now();
        auto activity_update = [&]
        {
            activity -= decay_per_second *
                std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - prev_measurement
                ).count();
            if (activity < 0.0) activity = 0.0;

            if (root()->needs_update()) activity += 2.0;
            if (in->is_key_input() ||
                in->is_mouse_input()) activity += 0.4;
            activity += 0.1 * animations();

            if (activity > 50.0) activity = 50.0;

            prev_measurement = std::chrono::steady_clock::now();
        };
        auto is_high_activity = [&]
        {
            return activity > 25.0;
        };
        auto is_low_activity = [&]
        {
            return activity < 5.0;
        };

        auto period_to_fps = [](std::chrono::milliseconds p)
        {
            auto ms = std::max<long long>(1, p.count());
            return 1000.0 / static_cast<double>(ms);
        };
        auto fps_to_period = [](double fps)
        {
            if (fps <= 0.0)
                fps = 1.0;
            auto ms = static_cast<long long>(1000.0 / fps);
            return std::chrono::milliseconds(std::max<long long>(1, ms));
        };

        if (_refresh_rate == std::chrono::milliseconds(0))
            set_refresh_rate(refresh_min);
        else
            _restore_refresh = refresh_min;

        auto user_min_fps = 0.0;
        auto fps_floor = 0.0;
        auto fps_cap = 0.0;
        auto refresh = _refresh_rate;
        auto fps_req = static_cast<std::size_t>(period_to_fps(refresh));
        auto last_measurement = std::chrono::steady_clock::now();

        auto clamp_refresh = [&](std::chrono::milliseconds r)
        {
            auto fps = period_to_fps(r);
            fps = std::clamp(fps, fps_floor, fps_cap);
            return fps_to_period(fps);
        };
        auto update_vars = [&]
        {
            user_min_fps = period_to_fps(refresh);
            fps_floor = std::max(1.0, user_min_fps);
            fps_cap = std::max(fps_floor * 3.0, 220.0);
        };

        if (_is_running)
            throw ScreenStartException{};
        _is_running = true;
        _is_suspended = false;
        _fps_avg = 0;

        std::size_t fps_counter = 0;

        _ctx->initialize();
        if (profile == Profile::Adaptive)
        {
            while (!_is_stop)
            {
                update_vars();
                const auto beg = std::chrono::steady_clock::now();
                activity_update();

                _frame();

                fps_counter++;
                if (beg - last_measurement >
                    std::chrono::seconds(1))
                {
                    _fps_avg = fps_counter;
                    fps_counter = 0;
                    last_measurement = std::chrono::steady_clock::now();
                }

                const auto end = std::chrono::steady_clock::now();
                const auto delta = end - beg;
                _prev_delta = std::chrono::duration_cast<std::chrono::microseconds>(delta);
                auto current_fps = period_to_fps(refresh);
                if (is_high_activity() && current_fps < fps_cap)
                {
                    current_fps = std::min(current_fps * growth_factor, fps_cap);
                    refresh = fps_to_period(current_fps);
                    fps_req = static_cast<std::size_t>(current_fps);
                }
                else if (is_low_activity() && current_fps > fps_floor)
                {
                    current_fps = std::max(current_fps * shrink_factor, fps_floor);
                    refresh = fps_to_period(current_fps);
                    fps_req = static_cast<std::size_t>(current_fps);
                }
                refresh = std::min(refresh, refresh);

                if (refresh > std::chrono::milliseconds(0))
                {
                    auto frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
                    auto sleep = refresh - frame_ms;
                    if (sleep > std::chrono::milliseconds(0))
                        _ctx->wait(sleep);
                }
            }
        }
        else if (profile == Profile::Stable)
        {
            while (!_is_stop)
            {
                const auto beg = std::chrono::steady_clock::now();

                _frame();

                fps_counter++;
                if (beg - last_measurement >
                    std::chrono::seconds(1))
                {
                    _fps_avg = fps_counter;
                    fps_counter = 0;
                    last_measurement = std::chrono::steady_clock::now();
                }

                const auto end = std::chrono::steady_clock::now();
                const auto delta = end - beg;
                _prev_delta = std::chrono::duration_cast<std::chrono::microseconds>(delta);

                auto frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
                auto sleep = _refresh_rate - frame_ms;
                _ctx->wait(
                    sleep <= std::chrono::milliseconds(0) ||
                    _refresh_rate == std::chrono::milliseconds(0) ?
                        std::chrono::milliseconds::max() : sleep
                );
            }
        }
        _ctx->deinitialize();
        _is_stop = false;

        ExtendedCodePage::deactivate_thread();
    }
    void Screen::stop_blocking()
    {
        _is_stop = true;
    }
    void Screen::set_refresh_rate(std::chrono::milliseconds refresh)
    {
        _refresh_rate = refresh;
    }

    void Screen::render()
    {
        if (!_is_suspended && root()->needs_update())
        {
            _signal(Event::PreRedraw);

            auto& root = *this->root().as();
            auto frame = root.frame();

            const auto [ bwidth, bheight ] = root.box();
            auto* output = _ctx->output();
            output->write(frame.data(), bwidth, bheight);

            _signal(Event::PostRedraw);
        }
    }
    void Screen::update()
    {
        root()->setcache(d2::Element::CachePolicy::Static);
        if (_ctx->input()->is_screen_resize())
        {
            _update_viewport();
        }
        if (_ctx->input()->is_mouse_input())
        {
            const auto target = _update_states(root(), _ctx->input()->mouse_position());
            const auto is_released = _ctx->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Release);
            const auto is_pressed = _ctx->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Press);

            // Autofocus

            auto uptarget = target;
            if (target != nullptr && is_pressed && !target->provides_cursor_sink() && target->parent() != nullptr)
                uptarget = _update_states_reverse(target->parent());

            auto _1 = _targetted.shared();
            auto _2 = _clicked.shared();
            auto _3 = _focused.shared();
            if (_targetted != target)
            {
                if (_targetted != nullptr)
                {
                    _targetted->setstate(Element::State::Hovered, false);
                    _trigger_hovered_events();
                }
                if (target != nullptr)
                    target->setstate(Element::State::Hovered, true);
                _targetted = target;
            }
            if (_clicked != uptarget)
            {
                if (_clicked != nullptr)
                    _clicked->setstate(Element::State::Clicked, false);
                _clicked = uptarget;
            }
            if (_focused != uptarget && is_pressed)
            {
                if (_focused != nullptr)
                {
                    _trigger_focused_events();
                }
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                }
                focus(uptarget);
            }
            if (_clicked != nullptr)
            {
                if (is_released)
                {
                    _clicked->setstate(Element::State::Clicked, false);
                }
                else if (is_pressed)
                {
                    _clicked->setstate(Element::State::Clicked, true);
                }
            }
        }
        if (_ctx->input()->is_key_input())
        {
            // Micro forwards
            if (_ctx->input()->is_pressed(sys::SystemInput::LeftControl) &&
                _ctx->input()->is_pressed(sys::SystemInput::key('W'), sys::SystemInput::KeyMode::Press))
            {
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                }
                else if (focused() != nullptr)
                {
                    _keynav_iterator = focused()->parent()->traverse().asp()->begin();
                    while (_keynav_iterator->traverse() != focused())
                        _keynav_iterator.increment();
                }
                _keynav_cycle();
                _keynav_iterator->setstate(Element::State::Keynavi, true);
                focus(_keynav_iterator->traverse());
            }
            // Micro reverse
            else if (_ctx->input()->is_pressed(sys::SystemInput::LeftControl) &&
                     _ctx->input()->is_pressed(sys::SystemInput::key('E'), sys::SystemInput::KeyMode::Press))
            {
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                }
                else if (focused() != nullptr)
                {
                    _keynav_iterator = focused()->parent()->traverse().asp()->begin();
                    while (_keynav_iterator->traverse() != focused())
                        _keynav_iterator.increment();
                }
                _keynav_cycle_reverse();
                _keynav_iterator->setstate(Element::State::Keynavi, true);
                focus(_keynav_iterator->traverse());
            }
            // Macro movement
            else if (_ctx->input()->is_pressed(sys::SystemInput::Shift) &&
                     _ctx->input()->is_pressed(sys::SystemInput::Tab, sys::SystemInput::KeyMode::Press))
            {
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                }
                else if (focused() != nullptr)
                {
                    _keynav_iterator = focused()->parent()->traverse().asp()->begin();
                    while (_keynav_iterator->traverse() != focused())
                        _keynav_iterator.increment();
                }
                _keynav_cycle_macro();
                _keynav_iterator->setstate(Element::State::Keynavi, true);
                focus(_keynav_iterator->traverse());
            }
            else if (_ctx->input()->is_pressed(sys::SystemInput::Escape, sys::SystemInput::KeyMode::Press))
            {
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                    focus(nullptr);
                }
                _keynav_iterator = nullptr;
            }
        }

        _run_interpolators();
        _trigger_events();

        _current->state->update();
    }

    Screen::eptr Screen::traverse()
    {
        return root();
    }
    Screen::eptr Screen::operator/(const std::string& path)
    {
        return eptr(
                   root()/path
               );
    }
}
