#include "d2_screen.hpp"
#include "d2_tree_parent.hpp"
#include "elements/d2_box.hpp"
#include "d2_exceptions.hpp"
#include <filesystem>

namespace d2::sys
{
    SystemScreen::Status SystemScreen::_load_impl()
    {
        _sig = context()->connect<Event, IOContext::ptr>();
        return Status::Ok;
    }
    SystemScreen::Status SystemScreen::_unload_impl()
    {
        _sig.disconnect();
        return Status::Ok;
    }

    void SystemScreen::_keynav_cycle_up(eptr ptr)
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
    void SystemScreen::_keynav_cycle()
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
    void SystemScreen::_keynav_cycle_up_reverse()
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
    void SystemScreen::_keynav_cycle_reverse()
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

    void SystemScreen::_keynav_cycle_macro()
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
    void SystemScreen::_keynav_cycle_macro_reverse()
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

    void SystemScreen::_run_interpolators()
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
    void SystemScreen::_trigger_focused(Event ev)
    {
        internal::ElementView::from(_focused).trigger_event(ev);
    }
    void SystemScreen::_trigger_hovered(Event ev)
    {
        internal::ElementView::from(_targetted).trigger_event(ev);
    }
    void SystemScreen::_trigger_events()
    {
        auto input = context()->input().ptr();
        const auto is_mouse_input = input->is_mouse_input();
        const auto is_keyboard_input = input->is_key_input();
        const auto is_key_seq_input = input->is_key_sequence_input();
        const auto is_resize = input->is_screen_resize();

        _signal(Event::Update);
        if (is_mouse_input) _signal(Event::MouseInput);
        if (is_resize) _signal(Event::Resize);
        if (is_keyboard_input) _signal(Event::KeyInput);
        if (is_key_seq_input) _signal(Event::KeySequenceInput);

        _trigger_focused_events();
        _trigger_hovered_events();
    }
    void SystemScreen::_trigger_focused_events()
    {
        auto input = context()->input().ptr();
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
    void SystemScreen::_trigger_hovered_events()
    {
        auto input = context()->input().ptr();
        const auto is_mouse_input = input->is_mouse_input();

        if (_targetted != nullptr && _targetted != _focused)
        {
            _trigger_hovered(Event::Update);
            if (is_mouse_input) _trigger_hovered(Event::MouseInput);
        }
    }
    void SystemScreen::_update_viewport()
    {
        const auto b = root().as<dx::Box>();
        b->accept_layout();
        const auto [ width, height ] = context()->input().ptr()->screen_size();
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
    SystemScreen::eptr SystemScreen::_update_states(eptr container, const std::pair<int, int>& mouse)
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
    SystemScreen::eptr SystemScreen::_update_states_reverse(eptr ptr)
    {
        if (ptr->provides_cursor_sink())
            return ptr;
        if (ptr->parent() != nullptr)
            return _update_states_reverse(ptr->parent());
        return nullptr;
    }

    void SystemScreen::_apply_impl(const Element::foreach_callback& func, eptr container) const
    {
        container.foreach([&func, this](eptr it)
        {
            func(it);
            if (it.is_type<ParentElement>())
                _apply_impl(func, it);
            return true;
        });
    }
    void SystemScreen::_signal(Event ev)
    {
        context()->signal(ev, context());
    }

    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name, const std::string& path)
    {
        const auto e = std::filesystem::path(name).extension();
        ModelType type = ModelType::Raw;
        if (e == "d2vm" || e == "ascii" || e == "txt") type = ModelType::Visual;
        else if (e == "d2m") type = ModelType::Raw;
        else D2_THRW_EX("Invalid model type", "Could not deduce type from file extension");
        return fetch_model(name, path, type);
    }
    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name, const std::string& path, ModelType type)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->load_model(path, type);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name, int width, int height, std::vector<Pixel> data)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset(std::move(data), width, height);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name, int width, int height, std::span<const Pixel> data)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset({ data.begin(), data.end() }, width, height);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name)
    {
        return _models[name];
    }
    void SystemScreen::remove_model(const std::string& name)
    {
        _models.erase(name);
    }

    TreeIter<ParentElement> SystemScreen::root() const
    {
        if (_current == nullptr)
            return nullptr;
        return _current->state->root();
    }

    void SystemScreen::set(const std::string& name)
    {
        D2_TLOG(Info, "Switching trees to: '", name, "'")
        if (_current != nullptr && name == root()->name())
            return;

        auto root = _trees[name];
        if (_current != nullptr)
        {
            if (_current->tags.tag<bool>("SwapOut"))
            {
                D2_TLOG(Verbose, "Swapping current tree out")
                _current->state->swap_out();
                this->root()->setstate(Element::Swapped, true);
                _current->swapped_out = true;
            }
            if (_current->tags.tag<bool>("SwapClean"))
            {
                D2_TLOG(Verbose, "Clearing current tree out")
                _current->state->root()->clear();
                _current->unbuilt = true;
            }
            if (const auto value = _current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
                value != std::chrono::milliseconds(0))
            {
                D2_TLOG(Verbose, "Restoring refresh rate to: ", _restore_refresh)
                set_refresh_rate(_restore_refresh);
            }
        }
        _current_name = name;
        _current = root;
        _focused = nullptr;

        if (_current->unbuilt)
        {
            D2_TLOG(Verbose, "Building new tree")
            _current->rebuild(
                this->root(), _current->state
            );
            _current->unbuilt = false;
        }
        if (_current->swapped_out)
        {
            D2_TLOG(Verbose, "Swapping in tree")
            root->state->swap_in();
            this->root()->setstate(Element::Swapped, false);
            _current->swapped_out = false;
        }

        if (const auto value = _current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
            value != std::chrono::milliseconds(0))
        {
            D2_TLOG(Verbose, "Overriding refresh rate to: ", value)
            _restore_refresh = _refresh_rate;
            set_refresh_rate(value);
        }

        D2_TLOG(Verbose, "Recycling")
        context()->input().ptr()->begincycle();
        context()->input().ptr()->endcycle();
        _update_viewport();

        D2_TLOG(Verbose, "Initializing tree")
        root->state->root()->initialize(true);
    }

    TreeTags& SystemScreen::tags()
    {
        return _current->tags;
    }
    TreeTags& SystemScreen::tags(const std::string& name)
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            D2_THRW("Invalid tree name");
        return f->second->tags;
    }

    const TreeTags& SystemScreen::tags() const
    {
        return _current->tags;
    }
    const TreeTags& SystemScreen::tags(const std::string& name) const
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            D2_THRW("Invalid tree name");
        return f->second->tags;
    }

    DynamicDependencyManager& SystemScreen::deps()
    {
        return _current->deps;
    }
    DynamicDependencyManager& SystemScreen::deps(const std::string& name)
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            D2_THRW("Invalid tree name");
        return f->second->deps;
    }

    const DynamicDependencyManager& SystemScreen::deps() const
    {
        return _current->deps;
    }
    const DynamicDependencyManager& SystemScreen::deps(const std::string& name) const
    {
        auto f = _trees.find(name);
        if (f == _trees.end())
            D2_THRW("Invalid tree name");
        return f->second->deps;
    }

    void SystemScreen::focus(eptr p)
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
    SystemScreen::eptr SystemScreen::focused() const
    {
        return _focused;
    }

    std::size_t SystemScreen::fps() const
    {
        return _fps_avg;
    }
    std::size_t SystemScreen::animations() const
    {
        if (_current == nullptr)
            return 0;
        return _current->interpolators.size();
    }
    std::chrono::microseconds SystemScreen::delta() const
    {
        return _prev_delta;
    }

    bool SystemScreen::is_keynav() const
    {
        return
            _keynav_iterator != nullptr &&
            _focused == _keynav_iterator.value();
    }

    void SystemScreen::apply(const Element::foreach_callback& func) const
    {
        _apply_impl(func, root());
    }
    void SystemScreen::apply_all(const Element::foreach_callback& func) const
    {
        for (decltype(auto) it : _trees)
            _apply_impl(func, it.second->state->root());
    }

    void SystemScreen::clear_animations(TreeIter<> ptr)
    {
        auto& interps = _current->interpolators;
        for (auto it = interps.begin(); it != interps.end();)
        {
            if ((*it)->target() == ptr.shared().get())
            {
                const auto saved = it;
                ++it;
                interps.erase(saved);
            }
            else ++it;
        }
    }

    void SystemScreen::erase_tree(const std::string& name)
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
    void SystemScreen::erase_tree()
    {
        erase_tree(_current_name);
    }
    void SystemScreen::clear_tree()
    {
        _trees.clear();
        _current = nullptr;
        _focused = nullptr;
        _current_name = "";
    }

    void SystemScreen::set_refresh_rate(std::chrono::milliseconds refresh)
    {
        _refresh_rate = refresh;
    }
    void SystemScreen::tick()
    {
        const auto ctx = context();
        // Update
        {
            root()->setcache(d2::Element::CachePolicy::Static);
            if (ctx->input().ptr()->is_screen_resize())
            {
                _update_viewport();
            }
            if (ctx->input().ptr()->is_mouse_input())
            {
                const auto target = _update_states(root(), ctx->input().ptr()->mouse_position());
                const auto is_released = ctx->input().ptr()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Release);
                const auto is_pressed = ctx->input().ptr()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Press);

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
            if (ctx->input().ptr()->is_key_input())
            {
                // Micro forwards
                if (ctx->input().ptr()->is_pressed(sys::SystemInput::LeftControl) &&
                    ctx->input().ptr()->is_pressed(sys::SystemInput::key('W'), sys::SystemInput::KeyMode::Press))
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
                else if (ctx->input().ptr()->is_pressed(sys::SystemInput::LeftControl) &&
                         ctx->input().ptr()->is_pressed(sys::SystemInput::key('E'), sys::SystemInput::KeyMode::Press))
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
                else if (ctx->input().ptr()->is_pressed(sys::SystemInput::Shift) &&
                         ctx->input().ptr()->is_pressed(sys::SystemInput::Tab, sys::SystemInput::KeyMode::Press))
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
                else if (ctx->input().ptr()->is_pressed(sys::SystemInput::Escape, sys::SystemInput::KeyMode::Press))
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
        const auto beg = std::chrono::steady_clock::now();
        // Render
        if (!ctx->is_suspended() && root()->needs_update())
        {
            _signal(Event::PreRedraw);

            auto& root = *this->root().as();
            auto frame = root.frame();

            const auto [ bwidth, bheight ] = root.box();
            auto output = ctx->output().ptr();
            output->write(frame.data(), bwidth, bheight);

            _signal(Event::PostRedraw);
        }
        {
            _fps_ctr++;
            if (beg - _last_mes >
                std::chrono::seconds(1))
            {
                _fps_avg = _fps_ctr;
                _fps_ctr = 0;
                _last_mes = std::chrono::steady_clock::now();
            }

            const auto end = std::chrono::steady_clock::now();
            const auto delta = end - beg;
            _prev_delta = std::chrono::duration_cast<std::chrono::microseconds>(delta);

            ctx->deadline(
                _refresh_rate -
                std::chrono::duration_cast<std::chrono::milliseconds>(delta)
            );
        }
    }

    SystemScreen::eptr SystemScreen::traverse()
    {
        return root();
    }
    SystemScreen::eptr SystemScreen::operator/(const std::string& path)
    {
        return eptr(
                   root()/path
               );
    }
}
