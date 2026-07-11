#include "core/screen/d2_screen.hpp"

#include <absl/strings/internal/str_format/extension.h>
#include <chrono>
#include <filesystem>

namespace d2::sys
{
    template<std::size_t First, std::size_t Len, std::size_t Count>
    void flip(std::bitset<Count>& bs)
    {
        if constexpr (First < Count && Len != 0)
        {
            constexpr std::size_t clamped = Len > Count - First ? Count - First : Len;
            static const auto mask = []()
            {
                std::bitset<Count> m;
                for (std::size_t i = 0; i < clamped; i++)
                    m.set(First + i);
                return m;
            }();

            bs ^= mask;
        }
    }

    template<SystemScreen::Event Ev, typename... Argv> void SystemScreen::_signal(Argv&&... args)
    {
        auto ctx = context();
        if constexpr (Ev == Event::KeyInput)
        {
            ctx->signal<Ev>(ref(*input()));
        }
        else if constexpr (Ev == Event::KeySequenceInput)
        {
            ctx->signal<Ev>(input()->sequence());
        }
        else if constexpr (Ev == Event::KeyMouseInput)
        {
            ctx->signal<Ev>(ref(*input()));
        }
        else if constexpr (Ev == Event::MouseMoved)
        {
            ctx->signal<Ev>(input()->mouse_position());
        }
        else if constexpr (Ev == Event::MouseInput)
        {
            ctx->signal<Ev>(ref(*input()));
        }
        else if constexpr (Ev == Event::Resize)
        {
            auto root = _ts.current->state->root();
            ctx->signal<Ev>(root->box());
        }
        else
            ctx->signal<Ev>(std::forward<Argv>(args)...);
    }

    SystemScreen::Status SystemScreen::_load_impl()
    {
        _sig = context()->connect<EventManifest>();
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

        const auto parent = _ts.keynav_iterator->parent();
        const auto start = _ts.keynav_iterator->traverse();
        do
        {
            _keynav_cycle();
            if (_ts.keynav_iterator->traverse() == start)
                break;
        } while (_ts.keynav_iterator->parent() == parent);
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

        const auto parent = _ts.keynav_iterator->parent();
        const auto start = _ts.keynav_iterator->traverse();
        do
        {
            _keynav_cycle_reverse();
            if (_ts.keynav_iterator->traverse() == start)
                break;
        } while (_ts.keynav_iterator->parent() == parent);
    }

    bool SystemScreen::_keynav_has_input() const
    {
        if (_ts.current == nullptr || root() == nullptr)
            return false;

        bool found = false;
        _apply_impl(
            [&](eptr it)
            {
                if (it->provides_input())
                    found = true;
                return true;
            },
            root()
        );
        return found;
    }

    bool SystemScreen::_keynav_prepare()
    {
        if (!_keynav_has_input())
            return false;

        if (_ts.keynav_iterator != nullptr)
        {
            _ts.keynav_iterator->setstate(Element::State::Keynavi, false);
            return true;
        }

        if (focused() != nullptr && focused()->parent() != nullptr)
        {
            _ts.keynav_iterator = focused()->parent()->traverse().asp()->begin();
            while (!_ts.keynav_iterator.is_end() && _ts.keynav_iterator->traverse() != focused())
                _ts.keynav_iterator.increment();

            if (!_ts.keynav_iterator.is_end())
                return true;

            _ts.keynav_iterator = nullptr;
        }

        return true;
    }

    void SystemScreen::_keynav_apply()
    {
        if (_ts.keynav_iterator == nullptr)
            return;

        _ts.keynav_iterator->setstate(Element::State::Keynavi, true);
        focus(_ts.keynav_iterator->traverse());
    }

    void SystemScreen::focus_next_minor(bool reverse)
    {
        if (!_keynav_prepare())
            return;

        if (reverse)
            _keynav_cycle_reverse();
        else
            _keynav_cycle();

        _keynav_apply();
    }

    void SystemScreen::focus_next_major(bool reverse)
    {
        if (!_keynav_prepare())
            return;

        if (reverse)
            _keynav_cycle_macro_reverse();
        else
            _keynav_cycle_macro();

        _keynav_apply();
    }

    void SystemScreen::clear_keynav()
    {
        if (_ts.keynav_iterator != nullptr)
            _ts.keynav_iterator->setstate(Element::State::Keynavi, false);

        _ts.keynav_iterator = nullptr;
        focus(nullptr);
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

    std::chrono::milliseconds SystemScreen::_run_animations()
    {
        std::chrono::milliseconds deadline{std::chrono::milliseconds::max()};
        auto& interps = _ts.current->animations;
        if (!interps.empty())
        {
            const auto end = interps.end();
            const auto beg = interps.begin();
            for (auto it = beg; it != end;)
            {
                auto& [anim, callback] = it->second;
                if (!anim->keep_alive())
                {
                    const auto saved = it;
                    ++it;
                    auto cb = std::move(callback);
                    auto target = d2::TreeIter{anim->target().first.lock()};
                    interps.erase(saved);
                    D2_SAFE_BLOCK_BEGIN
                    cb(target);
                    D2_SAFE_BLOCK_END
                    if (it == interps.end())
                        break;
                }
                else
                {
                    deadline = std::min(deadline, anim->update());
                    ++it;
                }
            }
        }
        return deadline;
    }
    void SystemScreen::_trigger_events()
    {
        auto input = context()->input().ptr();
        auto frame = input->frame();

        const auto is_mouse_input = frame->had_event(in::Event::KeyMouseInput);
        const auto is_keyboard_input = frame->had_event(in::Event::KeyInput);
        const auto is_key_seq_input = frame->had_event(in::Event::KeySequenceInput);
        const auto is_resize = frame->had_event(in::Event::ScreenResize);
        const auto is_mouse_moved = frame->had_event(in::Event::MouseMovement);

        auto view = in::internal::InputFrameView::from(frame.get());

        _frame = frame.get();

        _signal<Event::Update>();
        view.set_consume(true, std::this_thread::get_id());

        if (is_mouse_input)
            _signal<Event::MouseInput>();
        if (is_mouse_moved)
            _signal<Event::MouseMoved>();
        if (is_resize)
            _signal<Event::Resize>();
        if (is_keyboard_input)
            _signal<Event::KeyInput>();
        if (is_key_seq_input)
            _signal<Event::KeySequenceInput>();

        view.apply_consume();
        {
            _trigger_focused_events(*frame);
            _trigger_hovered_events(*frame);
        }
        view.set_consume(false, std::this_thread::get_id());
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
            static constexpr auto mouse_first = std::size_t(in::Mouse::Left);
            static constexpr auto mouse_last = std::size_t(in::Mouse::MouseKeyMax);

            in::InputFrame::keymap mask;
            flip<mouse_first, mouse_last - mouse_first>(mask);

            mask = in::internal::InputFrameView::from(&frame).mask_key_consume(mask);
            _trigger_focused_events(frame, _ts.focused);
            in::internal::InputFrameView::from(&frame).mask_key_consume(mask);
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
        const auto is_mouse_input = frame.had_event(in::Event::MouseMovement) ||
                                    frame.had_event(in::Event::KeyMouseInput) ||
                                    frame.had_event(in::Event::ScrollWheelMovement);
        if (is_mouse_input)
        {
            static constexpr auto mouse_first = std::size_t(in::Mouse::Left);

            in::InputFrame::keymap mask;
            flip<0, mouse_first>(mask);

            mask = in::internal::InputFrameView::from(&frame).mask_key_consume(mask);
            _trigger_hovered_events(frame, _ts.targetted);
            in::internal::InputFrameView::from(&frame).mask_key_consume(mask);
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
    SystemScreen::eptr SystemScreen::_update_states(eptr container, Position mouse)
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
                        if (mouse.x >= x && mouse.y >= y && mouse.x < (x + width) &&
                            mouse.y < (y + height))
                        {
                            mouse_target = it;
                            auto res = _update_states(it, {mouse.x - x, mouse.y - y});
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
        const std::string& name, int width, int height, std::vector<pixel> data
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
        const std::string& name, int width, int height, std::span<const pixel> data
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

    void SystemScreen::set(std::string_view name)
    {
        D2_TLOG(Info, "Switching trees to: '", name, "'")
        if (_ts.current != nullptr && name == root()->name())
            return;

        const auto prev = _ts.current_name;
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

        _signal<Event::TreeSwap>(std::string(prev), std::string(name));
    }

    TreeTags& SystemScreen::tags()
    {
        return _ts.current->tags;
    }
    DynamicDependencyManager& SystemScreen::deps()
    {
        return _ts.current->deps;
    }
    DynamicDependencyManager& SystemScreen::deps(std::string_view name)
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
        return _ts.current->animations.size();
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
        auto& interps = _ts.current->animations;
        for (auto it = interps.begin(); it != interps.end();)
        {
            auto& [anim, callback] = it->second;
            if (anim->target().first.lock() == ptr.shared())
            {
                const auto saved = it;
                ++it;
                auto cb = std::move(callback);
                interps.erase(saved);
                D2_SAFE_BLOCK_BEGIN
                cb(ptr);
                D2_SAFE_BLOCK_END
            }
            else
                ++it;
        }
    }
    void SystemScreen::clear_animations(TreeIter<> ptr, style::uai_property prop)
    {
        auto& interps = _ts.current->animations;
        auto f = interps.find(std::make_pair(ptr.shared().get(), prop));
        if (f != interps.end())
        {
            auto cb = std::move(f->second.callback);
            interps.erase(f);
            D2_SAFE_BLOCK_BEGIN
            cb(ptr);
            D2_SAFE_BLOCK_END
        }
    }

    std::string SystemScreen::name()
    {
        return _ts.current_name;
    }

    void SystemScreen::erase_tree(std::string_view name)
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
        const auto beg = std::chrono::steady_clock::now();
        const auto ctx = context();
        const auto input = ctx->input();
        std::chrono::steady_clock::time_point interp_deadline{
            std::chrono::steady_clock::time_point::max()
        };
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

            _ts.current->state->update();

            _trigger_events();

            const auto interp_ms = _run_animations();
            ctx->deadline(interp_ms == Animation::Wake::refresh ? _refresh_rate : interp_ms);

            if (input->had_pulse())
                ctx->deadline(std::chrono::milliseconds{0});
        }
        // Render
        if (!ctx->is_suspended() && root()->needs_update())
        {
            _signal<Event::PreRedraw>();

            auto& root = *this->root().as();
            auto frame = root.frame();

            const auto [bwidth, bheight] = root.box();
            auto output = ctx->output().ptr();
            output->write(frame.data(), bwidth, bheight);

            _signal<Event::PostRedraw>();
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
