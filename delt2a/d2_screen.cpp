#include "d2_screen.hpp"
#include "elements/d2_box.hpp"
#include "d2_exceptions.hpp"
#include <filesystem>

namespace d2
{
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
        std::unique_lock lock(_interp_mtx);
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
    void Screen::_trigger_focused(IOContext::Event ev)
    {
        _focused.as()->_trigger_event(ev);
    }
    void Screen::_trigger_hovered(IOContext::Event ev)
    {
        _targetted.as()->_trigger_event(ev);
    }
    void Screen::_trigger_events()
    {
        auto* input = _ctx->input();
        const bool is_mouse_input = input->is_mouse_input();
        const bool is_keyboard_input = input->is_key_input();
        const bool is_key_seq_input = input->is_key_sequence_input();
        const bool is_resize = input->is_screen_resize();

        _ctx->trigger<IOContext::Event::Update>(_current->state);
        if (is_mouse_input) _ctx->trigger<IOContext::Event::MouseInput>(_current->state);
        if (is_resize) _ctx->trigger<IOContext::Event::Resize>(_current->state);
        if (is_keyboard_input) _ctx->trigger<IOContext::Event::KeyInput>(_current->state);
        if (is_key_seq_input) _ctx->trigger<IOContext::Event::KeySequenceInput>(_current->state);

        _trigger_focused_events();
        _trigger_hovered_events();
    }
    void Screen::_trigger_focused_events()
    {
        auto* input = _ctx->input();
        const bool is_mouse_input = input->is_mouse_input();
        const bool is_keyboard_input = input->is_key_input();
        const bool is_key_seq_input = input->is_key_sequence_input();
        const bool is_resize = input->is_screen_resize();

        if (_focused != nullptr)
        {
            _trigger_focused(IOContext::Event::Update);
            if (is_mouse_input) _trigger_focused(IOContext::Event::MouseInput);
            if (is_resize) _trigger_focused(IOContext::Event::Resize);
            if (is_keyboard_input) _trigger_focused(IOContext::Event::KeyInput);
            if (is_key_seq_input) _trigger_focused(IOContext::Event::KeySequenceInput);
        }
    }
    void Screen::_trigger_hovered_events()
    {
        auto* input = _ctx->input();
        const bool is_mouse_input = input->is_mouse_input();

        if (_targetted != nullptr && _targetted != _focused)
        {
            _trigger_hovered(IOContext::Event::Update);
            if (is_mouse_input) _trigger_hovered(IOContext::Event::MouseInput);
        }
    }
    void Screen::_update_viewport()
    {
        const auto b = root().as<dx::Box>();
        b->accept_layout();
        const auto [ width, height ] = _ctx->input()->screen_size();
        const auto [ pwidth, pheight ] = b->box();
        if (pwidth != width || pheight != height)
        {
            b->override_dimensions(width, height);
            b->_signal_write(Element::WriteType::Style);
        }
    }
    Screen::eptr Screen::_update_states(eptr container, const std::pair<int, int>& mouse)
    {
        if (!container.is_type<ParentElement>())
            return nullptr;

        eptr mouse_target{ nullptr };
        // Press B to pass the vibe check
        container.asp()->foreach_internal([&](eptr it)
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
                        // Zindex lower/equal than -120 makes the child lose precedence over it's parent
                        mouse_target = it;
                        auto res = _update_states(it, std::pair(mouse.first - x, mouse.second - y));
                        if (res != nullptr && res->getzindex() > -120)
                            mouse_target = std::move(res);
                    }
                }
            }
        });
        return mouse_target;
    }

    void Screen::_apply_impl(const Element::foreach_callback& func, eptr container) const
    {
        container.foreach([&func, this](eptr it)
        {
            func(it);
            if (it.is_type<ParentElement>())
                _apply_impl(func, it);
        });
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
        std::unique_lock lock(_models_mtx);

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
        std::unique_lock lock(_models_mtx);

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
        std::unique_lock lock(_models_mtx);

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
        std::unique_lock lock(_models_mtx);
        return _models[name];
    }
    void Screen::remove_model(const std::string& name)
    {
        std::unique_lock lock(_models_mtx);
        _models.erase(name);
    }

    IOContext::ptr Screen::context() const noexcept
    {
        return _ctx;
    }
    Screen::eptr Screen::root() const noexcept
    {
        if (_current == nullptr)
            return nullptr;
        return _current->state->root();
    }

    void Screen::set(const std::string& name)
    {
        std::unique_lock lock(_mtx);

        if (_current != nullptr && name == root()->name())
            return;

        auto root = _trees[name];
        if (_current != nullptr)
        {
            _current->state->swap_out();
            this->root()->setstate(Element::Swapped, true);
        }
        _current_name = name;
        _current = root;
        _focused = nullptr;

        root->state->swap_in();
        this->root()->setstate(Element::Swapped, false);

        _ctx->input()->begincycle();
        _ctx->input()->endcycle();
        _update_viewport();

        root->state->root()->initialize(true);
    }

    void Screen::focus(eptr p)
    {
        if (p != _focused)
        {
            if (_focused != nullptr)
            {
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
    Screen::eptr Screen::focused() const noexcept
    {
        return _focused;
    }

    std::size_t Screen::fps() const noexcept
    {
        return _fps_avg.load();
    }
    std::size_t Screen::animations() const noexcept
    {
        std::unique_lock lock(_interp_mtx);
        if (_current == nullptr)
            return 0;
        return _current->interpolators.size();
    }
    std::chrono::microseconds Screen::delta() const noexcept
    {
        return _prev_delta;
    }

    bool Screen::is_keynav() const noexcept
    {
        std::unique_lock lock(_mtx);
        return
            _keynav_iterator != nullptr &&
            _focused == _keynav_iterator.value();
    }

    void Screen::apply(const Element::foreach_callback& func) const
    {
        std::unique_lock lock(_mtx);
        _apply_impl(func, root());
    }
    void Screen::apply_all(const Element::foreach_callback& func) const
    {
        std::unique_lock lock(_mtx);
        for (decltype(auto) it : _trees)
            _apply_impl(func, it.second->state->root());
    }

    void Screen::clear_animations(Element::ptr ptr) noexcept
    {
        std::unique_lock lock(_interp_mtx);
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
        std::unique_lock lock(_mtx);
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

    bool Screen::is_suspended() const noexcept
    {
        return _is_suspended;
    }
    void Screen::suspend(bool state) noexcept
    {
        _is_suspended = state;
    }

    void Screen::start_blocking(std::chrono::milliseconds refresh, Profile profile)
    {
        ExtendedCodePage::activate_thread();

        static constexpr auto factor = 1.5;
        auto update_req_fps = [&]() -> std::size_t
        {
            [[ unlikely ]] if (!refresh.count())
                return ~0ull;
            return 1000 / refresh.count();
        };

        if (_is_running)
            throw ScreenStartException{};
        _is_running = true;
        _is_suspended = false;
        _main_thread_id = std::this_thread::get_id();
        _fps_avg = 0;

        set_refresh_rate(refresh);
        std::size_t frames_wo_update = 0;
        std::size_t frames_w_update = 0;
        std::size_t fps = 0;
        auto fps_req = update_req_fps();
        auto last_measurement = std::chrono::high_resolution_clock::now();

        _worker = _ctx->scheduler()->make_cyclic_worker();
        while (!_is_stop)
        {
            const auto refresh_max = _refresh_rate.load();
            const auto beg = std::chrono::high_resolution_clock::now();

            if (root()->needs_update())
            {
                frames_wo_update = 0;
                frames_w_update++;
            }
            else
            {
                frames_wo_update++;
                frames_w_update = 0;
            }

            _frame();

            fps++;
            if (
                std::chrono::high_resolution_clock::now() - last_measurement >
                std::chrono::seconds(1)
            )
            {
                _fps_avg = fps;
                fps = 0;
                last_measurement = std::chrono::high_resolution_clock::now();
            }

            const auto end = std::chrono::high_resolution_clock::now();
            const auto delta = end - beg;
            _prev_delta = std::chrono::duration_cast<std::chrono::microseconds>(delta);

            if (profile == Profile::Adaptive)
            {
                if (frames_w_update > fps_req / 4)
                {
                    refresh = std::chrono::milliseconds(
                                  static_cast<std::size_t>(refresh.count() / factor)
                              );
                    fps_req = update_req_fps();
                }
                else if (frames_wo_update > fps_req * 2)
                {
                    refresh = std::min(refresh_max, std::chrono::milliseconds(
                                           static_cast<std::size_t>(refresh.count() * factor)
                                       ));
                    fps_req = update_req_fps();
                }
            }
            else refresh = refresh_max;
            if (refresh > std::chrono::seconds(0))
            {
                _worker.wait(
                    refresh -
                    std::chrono::duration_cast<std::chrono::milliseconds>(delta)
                );
            }
        }
        _is_stop = false;
        _worker.stop();
        _ctx->scheduler()->clear();

        ExtendedCodePage::deactivate_thread();
    }
    void Screen::stop_blocking() noexcept
    {
        _is_stop = true;
    }
    void Screen::set_refresh_rate(std::chrono::milliseconds refresh) noexcept
    {
        _refresh_rate = refresh;
    }

    void Screen::render()
    {
        if (!_is_suspended)
        {
            std::unique_lock lock(_mtx);
            if (root()->needs_update())
            {
                _ctx->trigger<IOContext::Event::PreRedraw>(_current->state);

                auto& root = *this->root().as();
                auto frame = root.frame();

                const auto [ bwidth, bheight ] = root.box();
                auto* output = _ctx->output();
                output->write(frame.data(), bwidth, bheight);
                _ctx->trigger<IOContext::Event::PostRedraw>(_current->state);
            }
        }
    }
    void Screen::update()
    {
        std::unique_lock lock(_mtx);

        if (_ctx->input()->is_screen_resize())
        {
            _update_viewport();
        }
        if (_ctx->input()->is_mouse_input())
        {
            const auto target = _update_states(root(), _ctx->input()->mouse_position());
            const auto is_released = _ctx->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left,
                                     sys::SystemInput::KeyMode::Release);
            const auto is_pressed = _ctx->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left,
                                    sys::SystemInput::KeyMode::Press);

            // Autofocus

            if (_targetted != target)
            {
                if (_targetted != nullptr)
                {
                    _targetted->setstate(Element::State::Clicked, false);
                    _targetted->setstate(Element::State::Hovered, false);
                    _trigger_hovered_events();
                }
                if (target != nullptr)
                    target->setstate(Element::State::Hovered, true);
                _targetted = target;
            }
            if (_targetted != nullptr)
            {
                if (is_released)
                {
                    _targetted->setstate(Element::State::Clicked, false);
                }
                else if (is_pressed)
                {
                    _targetted->setstate(Element::State::Clicked);
                }
            }
            if (_focused != target && is_pressed)
            {
                if (_focused != nullptr)
                {
                    _trigger_focused_events();
                }
                if (_keynav_iterator != nullptr)
                {
                    _keynav_iterator->setstate(Element::State::Keynavi, false);
                }
                focus(target);
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
