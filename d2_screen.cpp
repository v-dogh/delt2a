#include "d2_screen.hpp"

#include <d2_exceptions.hpp>
#include <d2_input_base.hpp>
#include <d2_tree_parent.hpp>
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
            _ts.keynav_iterator = root().asp()->begin();
        }
        else
        {
            _ts.keynav_iterator = parent->traverse().asp()->begin();
            while (_ts.keynav_iterator->traverse() != cparent)
                _ts.keynav_iterator.increment();

            if (_ts.keynav_iterator.is_end())
            {
                _keynav_cycle_up(cparent);
            }
            else
            {
                _ts.keynav_iterator.increment();
            }
            if (_ts.keynav_iterator.is_end())
            {
                _keynav_cycle_up(cparent);
            }
        }
    }
    void SystemScreen::_keynav_cycle()
    {
        if (_ts.keynav_iterator == nullptr)
        {
            _ts.keynav_iterator = root().asp()->begin();
            if (_ts.keynav_iterator->provides_input())
                return;
        }

        do
        {
            if (_ts.keynav_iterator->traverse().is_type<ParentElement>() &&
                !_ts.keynav_iterator->traverse().asp()->empty())
            {
                _ts.keynav_iterator = _ts.keynav_iterator->traverse().asp()->begin();
            }
            else
            {
                eptr ptr = _ts.keynav_iterator->traverse();
                _ts.keynav_iterator.increment();
                if (_ts.keynav_iterator.is_end())
                {
                    _keynav_cycle_up(ptr);
                }
            }
        } while (!_ts.keynav_iterator->provides_input());
    }
    void SystemScreen::_keynav_cycle_up_reverse()
    {
        const auto cparent = _ts.keynav_iterator->parent();
        const auto parent = cparent->parent();

        if (parent == nullptr)
        {
            _ts.keynav_iterator = root().asp()->end();
            _ts.keynav_iterator.decrement();
        }
        else
        {
            _ts.keynav_iterator = parent->traverse().asp()->begin();
            while (_ts.keynav_iterator->traverse() != cparent)
                _ts.keynav_iterator.increment();

            if (_ts.keynav_iterator.is_begin())
            {
                _keynav_cycle_up_reverse();
            }
            else
            {
                _ts.keynav_iterator.decrement();
            }
        }
    }
    void SystemScreen::_keynav_cycle_reverse()
    {
        if (_ts.keynav_iterator == nullptr)
        {
            _ts.keynav_iterator = root().asp()->end();
            _ts.keynav_iterator.decrement();
            if (_ts.keynav_iterator->provides_input())
                return;
        }

        do
        {
            if (_ts.keynav_iterator->traverse().is_type<ParentElement>() &&
                !_ts.keynav_iterator->traverse().asp()->empty())
            {
                _ts.keynav_iterator = _ts.keynav_iterator->traverse().asp()->end();
                _ts.keynav_iterator.decrement();
            }
            else
            {
                if (_ts.keynav_iterator.is_begin())
                {
                    _keynav_cycle_up_reverse();
                }
                else
                {
                    _ts.keynav_iterator.decrement();
                }
            }
        } while (!_ts.keynav_iterator->provides_input());
    }

    void SystemScreen::_keynav_cycle_macro()
    {
        if (_ts.keynav_iterator == nullptr)
        {
            _ts.keynav_iterator = root().asp()->begin();
            if (_ts.keynav_iterator->provides_input())
                return;
        }

        auto parent = _ts.keynav_iterator->parent();
        do
            _keynav_cycle();
        while (_ts.keynav_iterator->parent() == parent);
    }
    void SystemScreen::_keynav_cycle_macro_reverse()
    {
        if (_ts.keynav_iterator == nullptr)
        {
            _ts.keynav_iterator = root().asp()->end();
            _ts.keynav_iterator.decrement();
            if (_ts.keynav_iterator->provides_input())
                return;
        }

        auto parent = _ts.keynav_iterator->parent();
        do
            _keynav_cycle_reverse();
        while (_ts.keynav_iterator->parent() == parent);
    }

    SystemScreen::eptr SystemScreen::_lca(eptr a, eptr b)
    {
        if (a == nullptr || b == nullptr)
            return nullptr;
        while (a != nullptr && a->depth() > b->depth())
            a = a->parent();
        while (b != nullptr && b->depth() > a->depth())
            b = b->parent();
        while (a != b && a != nullptr && b != nullptr)
        {
            a = a->parent();
            b = b->parent();
        }
        return a;
    }

    void SystemScreen::_run_interpolators()
    {
        auto& interps = _ts.current->interpolators;
        if (!interps.empty())
        {
            const auto end = interps.end();
            const auto beg = interps.begin();
            for (auto it = beg; it != end;)
            {
                if ((*it)->getowner() == std::hash<tree>()(_ts.current))
                {
                    if (!(*it)->keep_alive())
                    {
                        const auto saved = it;
                        ++it;
                        interps.erase(saved);
                        if (it == interps.end())
                            break;
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
    void SystemScreen::_trigger_events()
    {
        auto input = context()->input().ptr();
        const auto is_mouse_input = input->had_event(in::Event::KeyMouseInput);
        const auto is_keyboard_input = input->had_event(in::Event::KeyInput);
        const auto is_key_seq_input = input->had_event(in::Event::KeySequenceInput);
        const auto is_resize = input->had_event(in::Event::ScreenResize);
        const auto is_mouse_moved = input->had_event(in::Event::MouseMovement);

        // I think this should be explicit (for top level events, whether the event consumes or not)
        auto frame = input->frame();
        in::internal::InputFrameView::from(frame.get())
            .set_consume(true, std::this_thread::get_id());
        _frame = frame.get();
        _signal(Event::Update);
        if (is_mouse_input)
            _signal(Event::MouseInput);
        if (is_mouse_moved)
            _signal(Event::MouseMoved);
        if (is_resize)
            _signal(Event::Resize);
        if (is_keyboard_input)
            _signal(Event::KeyInput);
        if (is_key_seq_input)
            _signal(Event::KeySequenceInput);
        in::internal::InputFrameView::from(frame.get()).apply_consume();
        {
            _trigger_focused_events(*frame);
            _trigger_hovered_events(*frame);
        }
        in::internal::InputFrameView::from(frame.get())
            .set_consume(false, std::this_thread::get_id());
        _frame = nullptr;
    }
    void SystemScreen::_trigger_focused_events(in::InputFrame& frame, eptr ptr)
    {
        if (ptr != nullptr)
        {
            internal::ElementView::from(ptr).trigger_event(frame, ptr != _ts.focused);
            _trigger_focused_events(frame, ptr->parent());
        }
    }
    void SystemScreen::_trigger_focused_events(in::InputFrame& frame)
    {
        const auto is_keyboard_input =
            frame.had_event(in::Event::KeyInput) || frame.had_event(in::Event::KeySequenceInput);
        if (is_keyboard_input)
        {
            in::InputFrame::keyboard_keymap mask;
            mask.flip();
            mask = in::internal::InputFrameView::from(&frame).mask_mouse_consume(mask);
            _trigger_focused_events(frame, _ts.focused);
            in::internal::InputFrameView::from(&frame).mask_mouse_consume(mask);
        }
    }
    void SystemScreen::_trigger_hovered_events(in::InputFrame& frame, eptr ptr)
    {
        if (ptr != nullptr)
        {
            internal::ElementView::from(ptr).trigger_event(frame, ptr != _ts.targetted);
            _trigger_hovered_events(frame, ptr->parent());
        }
    }
    void SystemScreen::_trigger_hovered_events(in::InputFrame& frame)
    {
        auto input = context()->input().ptr();
        const auto is_mouse_input = input->had_event(in::Event::KeyMouseInput) ||
                                    input->had_event(in::Event::ScrollWheelMovement);
        if (is_mouse_input)
        {
            in::InputFrame::keyboard_keymap mask;
            mask.flip();
            mask = in::internal::InputFrameView::from(&frame).mask_keyboard_consume(mask);
            _trigger_hovered_events(frame, _ts.targetted);
            in::internal::InputFrameView::from(&frame).mask_keyboard_consume(mask);
        }
    }
    void SystemScreen::_trigger_rc_hover_events(eptr n, eptr o)
    {
        const auto c = _lca(o, n);
        for (auto x = o; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcHover, false);
        for (auto x = n; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcHover, true);
    }
    void SystemScreen::_trigger_rc_click_events(eptr n, eptr o)
    {
        const auto c = _lca(o, n);
        for (auto x = o; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcClick, false);
        for (auto x = n; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcClick, true);
    }
    void SystemScreen::_trigger_rc_focus_events(eptr n, eptr o)
    {
        const auto c = _lca(o, n);
        for (auto x = o; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcFocus, false);
        for (auto x = n; x != nullptr && x != c; x = x->parent())
            x->setstate(Element::State::RcFocus, true);
    }

    void SystemScreen::_update_viewport()
    {
        const auto b = root().as<d2::ParentElement>();
        b->accept_layout();
        const auto [width, height] = context()->input().ptr()->screen_size();
        const auto [pwidth, pheight] = b->box();
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
    SystemScreen::eptr
    SystemScreen::_update_states(eptr container, const std::pair<int, int>& mouse)
    {
        if (!container.is_type<ParentElement>())
            return nullptr;

        eptr mouse_target{nullptr};
        // Press B to pass the vibe check
        container.asp()->foreach_internal(
            [&](eptr it) -> bool
            {
                if (it->getstate(Element::State::Display))
                {
                    const auto [width, height] = it->box();
                    const auto [x, y] = it->position();
                    if (mouse_target == nullptr || it->getzindex() > mouse_target->getzindex())
                    {
                        if (mouse.first >= x && mouse.second >= y && mouse.first < (x + width) &&
                            mouse.second < (y + height))
                        {
                            mouse_target = it;
                            auto res =
                                _update_states(it, std::pair(mouse.first - x, mouse.second - y));
                            if (res != nullptr && res->getzindex() > d2::ParentElement::underlap)
                                mouse_target = std::move(res);
                        }
                    }
                }
                return true;
            }
        );
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
        container.foreach (
            [&func, this](eptr it)
            {
                func(it);
                if (it.is_type<ParentElement>())
                    _apply_impl(func, it);
                return true;
            }
        );
    }
    void SystemScreen::_signal(Event ev)
    {
        context()->signal(ev, context());
    }

    MatrixModel::ptr SystemScreen::fetch_model(const std::string& name, const std::string& path)
    {
        const auto e = std::filesystem::path(name).extension();
        ModelType type = ModelType::Raw;
        if (e == "d2vm" || e == "ascii" || e == "txt")
            type = ModelType::Visual;
        else if (e == "d2m")
            type = ModelType::Raw;
        else
            D2_THRW_EX("Invalid model type", "Could not deduce type from file extension");
        return fetch_model(name, path, type);
    }
    MatrixModel::ptr
    SystemScreen::fetch_model(const std::string& name, const std::string& path, ModelType type)
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->load_model(path, type);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr SystemScreen::fetch_model(
        const std::string& name, int width, int height, std::vector<Pixel> data
    )
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset(std::move(data), width, height);
        _models[name] = ptr;
        return ptr;
    }
    MatrixModel::ptr SystemScreen::fetch_model(
        const std::string& name, int width, int height, std::span<const Pixel> data
    )
    {
        auto f = _models.find(name);
        if (f != _models.end() && f->second != nullptr)
            return f->second;

        auto ptr = std::make_shared<MatrixModel>();
        ptr->reset({data.begin(), data.end()}, width, height);
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
        if (_ts.current == nullptr)
            return nullptr;
        return _ts.current->state->root();
    }

    void SystemScreen::set(const std::string& name)
    {
        D2_TLOG(Info, "Switching trees to: '", name, "'")
        if (_ts.current != nullptr && name == root()->name())
            return;

        auto root = _trees[name];
        if (_ts.current != nullptr)
        {
            if (_ts.current->tags.tag<bool>("SwapOut"))
            {
                D2_TLOG(Verbose, "Swapping current tree out")
                _ts.current->state->swap_out();
                this->root()->setstate(Element::Swapped, true);
                _ts.current->swapped_out = true;
            }
            if (_ts.current->tags.tag<bool>("SwapClean"))
            {
                D2_TLOG(Verbose, "Clearing current tree out")
                _ts.current->state->root()->clear();
                _ts.current->unbuilt = true;
            }
            if (const auto value =
                    _ts.current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
                value != std::chrono::milliseconds(0))
            {
                D2_TLOG(Verbose, "Restoring refresh rate to: ", _restore_refresh)
                set_refresh_rate(_restore_refresh);
            }
        }
        _ts.current_name = name;
        _ts.current = root;
        _ts.focused = nullptr;

        if (_ts.current->unbuilt)
        {
            D2_TLOG(Verbose, "Building new tree")
            _ts.current->rebuild(this->root(), _ts.current->state);
            _ts.current->unbuilt = false;
        }
        if (_ts.current->swapped_out)
        {
            D2_TLOG(Verbose, "Swapping in tree")
            root->state->swap_in();
            this->root()->setstate(Element::Swapped, false);
            _ts.current->swapped_out = false;
        }

        if (const auto value = _ts.current->tags.tag<std::chrono::milliseconds>("OverrideRefresh");
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
        return _ts.current->tags;
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
        return _ts.current->tags;
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
        return _ts.current->deps;
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
        return _ts.current->deps;
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
        if (p != _ts.focused)
        {
            const auto cpy_focused = _ts.focused;
            if (_ts.focused != nullptr)
            {
                auto _ = _ts.focused.shared();
                _ts.focused->setstate(Element::Focused, false);
                _ts.focused->setstate(Element::State::Clicked, false);
                _ts.focused->setstate(Element::State::Hovered, false);
            }
            {
                _ts.focused = p;
                if (p != nullptr)
                    _ts.focused->setstate(Element::Focused);
            }
            _trigger_rc_focus_events(p, cpy_focused);
        }
    }
    SystemScreen::eptr SystemScreen::focused() const
    {
        return _ts.focused;
    }
    SystemScreen::eptr SystemScreen::clicked() const
    {
        return _ts.clicked;
    }
    SystemScreen::eptr SystemScreen::hovered() const
    {
        return _ts.targetted;
    }

    std::size_t SystemScreen::fps() const
    {
        return _fps_avg;
    }
    std::size_t SystemScreen::animations() const
    {
        if (_ts.current == nullptr)
            return 0;
        return _ts.current->interpolators.size();
    }
    std::chrono::microseconds SystemScreen::delta() const
    {
        return _prev_delta;
    }

    bool SystemScreen::is_keynav() const
    {
        return _ts.keynav_iterator != nullptr && _ts.focused == _ts.keynav_iterator.value();
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
        auto& interps = _ts.current->interpolators;
        for (auto it = interps.begin(); it != interps.end();)
        {
            if ((*it)->target() == ptr.shared().get())
            {
                const auto saved = it;
                ++it;
                interps.erase(saved);
            }
            else
                ++it;
        }
    }

    void SystemScreen::erase_tree(const std::string& name)
    {
        auto root = _trees[name];
        if (root == _ts.current)
        {
            _ts.focused = nullptr;
            _ts.current = nullptr;
            _ts.current_name = "";
        }
        _trees.erase(name);
    }
    void SystemScreen::erase_tree()
    {
        erase_tree(_ts.current_name);
    }
    void SystemScreen::clear_tree()
    {
        _trees.clear();
        _ts.current = nullptr;
        _ts.focused = nullptr;
        _ts.current_name = "";
    }

    in::InputFrame* SystemScreen::input()
    {
        if (_frame == nullptr)
            D2_THRW("Cannot access input frame outside of screen events");
        return _frame;
    }

    void SystemScreen::set_refresh_rate(std::chrono::milliseconds refresh)
    {
        _refresh_rate = refresh;
    }
    void SystemScreen::tick()
    {
        const auto ctx = context();
        const auto input = ctx->input();
        // Update
        {
            root()->setcache(d2::Element::CachePolicy::Static);
            if (input->had_event(in::Event::ScreenResize))
            {
                _update_viewport();
            }
            if (input->had_event(in::Event::MouseMovement) ||
                input->had_event(in::Event::KeyMouseInput))
            {
                const auto target = _update_states(root(), ctx->input().ptr()->mouse_position());
                const auto is_released = input->active(in::mouse::Left, in::mode::Release);
                const auto is_pressed = input->active(in::mouse::Left, in::mode::Press);
                const auto cpy_targetted = _ts.targetted;

                auto uptarget = target;
                if (target != nullptr && is_pressed && !target->provides_cursor_sink() &&
                    target->parent() != nullptr)
                {
                    uptarget = _update_states_reverse(target->parent());
                }

                auto _1 = _ts.targetted.shared();
                auto _2 = _ts.clicked.shared();
                auto _3 = _ts.focused.shared();
                if (_ts.targetted != target)
                {
                    if (_ts.targetted != nullptr)
                        _ts.targetted->setstate(Element::State::Hovered, false);

                    if (target != nullptr)
                        target->setstate(Element::State::Hovered, true);

                    _ts.targetted = target;
                    _trigger_rc_hover_events(target, cpy_targetted);
                }
                if (is_pressed)
                {
                    const auto old_clicked = _ts.clicked;
                    if (_ts.clicked != uptarget)
                    {
                        if (_ts.clicked != nullptr)
                            _ts.clicked->setstate(Element::State::Clicked, false);
                        _ts.clicked = uptarget;
                        if (_ts.clicked != nullptr)
                            _ts.clicked->setstate(Element::State::Clicked, true);
                        _trigger_rc_click_events(_ts.clicked, old_clicked);
                    }
                    else if (_ts.clicked != nullptr)
                    {
                        _ts.clicked->setstate(Element::State::Clicked, true);
                    }
                }
                if (is_released)
                {
                    const auto old_clicked = _ts.clicked;

                    if (_ts.clicked != nullptr)
                        _ts.clicked->setstate(Element::State::Clicked, false);
                    _ts.clicked = nullptr;
                    _trigger_rc_click_events(nullptr, old_clicked);
                }
                if (_ts.focused != uptarget && is_pressed)
                {
                    if (_ts.keynav_iterator != nullptr)
                        _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
                    focus(uptarget);
                }
            }
            if (input->had_event(in::Event::KeyInput))
            {
                // Micro forwards
                if (input->active(in::special::LeftControl, in::mode::Hold) &&
                    input->active(in::key('W'), in::mode::Press))
                {
                    if (_ts.keynav_iterator != nullptr)
                    {
                        _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
                    }
                    else if (focused() != nullptr)
                    {
                        _ts.keynav_iterator = focused()->parent()->traverse().asp()->begin();
                        while (_ts.keynav_iterator->traverse() != focused())
                            _ts.keynav_iterator.increment();
                    }
                    _keynav_cycle();
                    _ts.keynav_iterator->setstate(Element::State::Keynavi, true);
                    focus(_ts.keynav_iterator->traverse());
                }
                // Micro reverse
                else if (
                    input->active(in::special::LeftControl, in::mode::Hold) &&
                    input->active(in::key('E'), in::mode::Press)
                )
                {
                    if (_ts.keynav_iterator != nullptr)
                    {
                        _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
                    }
                    else if (focused() != nullptr)
                    {
                        _ts.keynav_iterator = focused()->parent()->traverse().asp()->begin();
                        while (_ts.keynav_iterator->traverse() != focused())
                            _ts.keynav_iterator.increment();
                    }
                    _keynav_cycle_reverse();
                    _ts.keynav_iterator->setstate(Element::State::Keynavi, true);
                    focus(_ts.keynav_iterator->traverse());
                }
                // Macro movement
                else if (
                    input->active(in::special::Shift, in::mode::Hold) &&
                    input->active(in::special::Tab, in::mode::Press)
                )
                {
                    if (_ts.keynav_iterator != nullptr)
                    {
                        _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
                    }
                    else if (focused() != nullptr)
                    {
                        _ts.keynav_iterator = focused()->parent()->traverse().asp()->begin();
                        while (_ts.keynav_iterator->traverse() != focused())
                            _ts.keynav_iterator.increment();
                    }
                    _keynav_cycle_macro();
                    _ts.keynav_iterator->setstate(Element::State::Keynavi, true);
                    focus(_ts.keynav_iterator->traverse());
                }
                else if (input->active(in::special::Escape, in::mode::Press))
                {
                    if (_ts.keynav_iterator != nullptr)
                    {
                        _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
                        focus(nullptr);
                    }
                    _ts.keynav_iterator = nullptr;
                }
            }

            _run_interpolators();
            _trigger_events();

            _ts.current->state->update();
        }
        const auto beg = std::chrono::steady_clock::now();
        // Render
        if (!ctx->is_suspended() && root()->needs_update())
        {
            _signal(Event::PreRedraw);

            auto& root = *this->root().as();
            auto frame = root.frame();

            const auto [bwidth, bheight] = root.box();
            auto output = ctx->output().ptr();
            output->write(frame.data(), bwidth, bheight);

            _signal(Event::PostRedraw);
        }
        {
            _fps_ctr++;
            if (beg - _last_mes > std::chrono::seconds(1))
            {
                _fps_avg = _fps_ctr;
                _fps_ctr = 0;
                _last_mes = std::chrono::steady_clock::now();
            }

            const auto end = std::chrono::steady_clock::now();
            const auto delta = end - beg;
            _prev_delta = std::chrono::duration_cast<std::chrono::microseconds>(delta);

            if (_refresh_rate != std::chrono::milliseconds::max())
                ctx->deadline(
                    _refresh_rate - std::chrono::duration_cast<std::chrono::milliseconds>(delta)
                );
        }
    }

    SystemScreen::eptr SystemScreen::traverse()
    {
        return root();
    }
    SystemScreen::eptr SystemScreen::operator/(const std::string& path)
    {
        return eptr(root() / path);
    }
} // namespace d2::sys
