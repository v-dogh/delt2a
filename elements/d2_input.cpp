#include "d2_input.hpp"
#include <d2_screen.hpp>
#include <elements/d2_element_utils.hpp>
#include <limits>
#include <mods/d2_ext.hpp>

namespace d2::dx
{
    void Input::_write_ptr(Pixel& px)
    {
        if ((data::input_options & InputOptions::Blink) && !ptr_blink_)
        {
            px.v = ' ';
        }
        else
        {
            px = Pixel::combine(
                (data::input_options & data::AutoPtrColor)
                    ? data::foreground_color.extend(data::ptr_color.v)
                    : data::ptr_color,
                data::background_color
            );
        }
    }

    bool Input::_is_highlighted()
    {
        return highlight_start_ != highlight_end_;
    }
    void Input::_remove_highlighted()
    {
        if (_is_highlighted())
        {
            const auto lower = std::min(highlight_start_, highlight_end_);
            const auto upper = std::max(highlight_start_, highlight_end_);
            const auto diff = upper - lower;
            data::text.erase(lower, diff);
            if (ptr_ >= upper)
                _move_left(diff);
            _reset_highlight();
        }
    }
    void Input::_begin_highlight_if()
    {
        if (highlight_start_ == highlight_end_)
        {
            highlight_start_ = ptr_;
            highlight_end_ = ptr_;
            _signal_write(WriteType::Style);
        }
    }
    void Input::_reset_highlight()
    {
        if (_is_highlighted())
        {
            highlight_start_ = 0;
            highlight_end_ = 0;
            _signal_write(WriteType::Style);
        }
    }
    void Input::_reset_ptrs()
    {
        ptr_ = 0;
        bound_lower_ = 0;
        highlight_start_ = 0;
        highlight_end_ = 0;
        _reset_highlight();
    }
    string Input::_highlighted_value()
    {
        if (_is_highlighted())
        {
            const auto lower = std::min(highlight_start_, highlight_end_);
            const auto upper = std::max(highlight_start_, highlight_end_);
            return data::text.substr(lower, upper - lower);
        }
        return "";
    }

    void Input::_start_blink()
    {
        if ((data::input_options & InputOptions::Blink) && getstate(State::Focused) &&
            btask_ == nullptr)
        {
            btask_ = context()->scheduler()->launch_cyclic(
                std::chrono::milliseconds(500),
                [this](auto)
                {
                    ptr_blink_ = !ptr_blink_;
                    _signal_write(WriteType::Style);
                }
            );
        }
    }
    void Input::_stop_blink()
    {
        if (btask_ != nullptr)
        {
            btask_.discard();
            btask_.sync_value();
            btask_ = nullptr;
        }
        ptr_blink_ = false;
        _signal_write(WriteType::Style);
    }
    void Input::_freeze_blink()
    {
        if (btask_ != nullptr)
        {
            btask_.discard();
            btask_ = nullptr;
        }
    }
    void Input::_unfreeze_blink()
    {
        if (ptr_blink_ && btask_ == nullptr)
            _start_blink();
    }

    void Input::_move_right(std::size_t cnt)
    {
        _reset_highlight();

        ptr_ += cnt;
        if (ptr_ > data::text.size())
        {
            ptr_ = data::text.size();
        }

        const auto upper = bound_lower_ + box().width;
        if (ptr_ >= upper && bound_lower_ < data::text.size())
        {
            const auto diff = ptr_ - upper;
            bound_lower_ += diff;
        }

        _signal_write(WriteType::Style);
    }
    void Input::_move_left(std::size_t cnt)
    {
        _reset_highlight();

        if (ptr_ >= cnt)
        {
            ptr_ -= cnt;
        }
        else
        {
            ptr_ = 0;
        }

        if (ptr_ < bound_lower_ && bound_lower_ > 0)
        {
            const auto diff = bound_lower_ - ptr_;
            bound_lower_ -= diff;
        }

        _signal_write(WriteType::Style);
    }

    bool Input::_provides_input_impl() const
    {
        return true;
    }

    Unit Input::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X:
            return data::x;
        case Element::Layout::Y:
            return data::y;
        case Element::Layout::Width:
            if (data::width.getunits() == Unit::Auto)
                return Unit(
                    int(data::pre.size() + std::max(data::hint.size(), data::text.size())) +
                    data::width.raw()
                );
            return data::width;
        case Element::Layout::Height:
            if (data::height.getunits() == Unit::Auto)
                return Unit(1 + data::height.raw());
            return data::height;
        }
    }

    void Input::_state_change_impl(State state, bool value)
    {
        if (state == State::Focused)
        {
            if (value)
            {
                _start_blink();
            }
            else
            {
                _stop_blink();
                _reset_highlight();
            }
        }
        else if (state == State::Clicked && value)
        {
            const auto [x, y] = mouse_object_space();
            if (x >= data::pre.size())
                ptr_ = bound_lower_ + (x - data::pre.size());
            _reset_highlight();
            _begin_highlight_if();
        }
        else if (state == State::Swapped)
        {
            if (value)
            {
                _freeze_blink();
            }
            else
            {
                _unfreeze_blink();
            }
        }
    }
    void Input::_event_impl(in::InputFrame& frame)
    {
        write_flag flag = WriteType::Style;
        bool write = false;

        if (frame.had_event(in::Event::MouseMovement) || frame.had_event(in::Event::KeyMouseInput))
        {
            if (frame.active(in::mouse::Left, in::mode::Hold))
            {
                const auto [x, y] = mouse_object_space();
                if (x >= data::pre.size())
                {
                    highlight_end_ = bound_lower_ + (x - data::pre.size());
                    write = true;
                }
            }
        }
        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::Enter, in::mode::Hold) &&
                !(data::input_options & ReadOnly))
            {
                if (!data::text.empty())
                {
                    string cpy = std::move(data::text);
                    data::text = std::string();
                    if (data::on_submit != nullptr)
                        data::on_submit(
                            std::static_pointer_cast<Input>(shared_from_this()), std::move(cpy)
                        );
                    _reset_ptrs();
                    flag |= WriteType::Offset;
                    write = true;
                }
            }
            else if (
                frame.active(in::special::Backspace, in::mode::Hold) &&
                !(data::input_options & ReadOnly)
            )
            {
                if (_is_highlighted())
                {
                    _remove_highlighted();
                }
                else if (!data::text.empty() && ptr_)
                {
                    data::text.erase(ptr_ - 1, 1);
                    if (ptr_ == bound_lower_)
                        bound_lower_--;
                    ptr_--;
                    write = true;
                }
                flag |= WriteType::Offset;
            }
            else if (
                frame.active(in::special::Delete, in::mode::Hold) &&
                !(data::input_options & ReadOnly)
            )
            {
                if (_is_highlighted())
                {
                    _remove_highlighted();
                }
                else if (ptr_ < data::text.size())
                {
                    data::text.erase(ptr_, 1);
                    write = true;
                }
                flag |= WriteType::Offset;
            }
            else if (frame.active(in::special::ArrowLeft, in::mode::Hold))
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    if (highlight_end_ > 0)
                    {
                        highlight_end_--;
                        write = true;
                    }
                }
                else
                    _move_left();
            }
            else if (frame.active(in::special::ArrowRight, in::mode::Hold))
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    if (highlight_end_ < data::text.size())
                    {
                        highlight_end_++;
                        if (highlight_end_ > bound_lower_ + box().width)
                            bound_lower_++;
                        write = true;
                    }
                }
                else
                    _move_right();
            }
            else if (frame.active(in::special::Escape, in::mode::Hold))
            {
                screen()->focus(nullptr);
            }
            else if (frame.active(in::special::LeftControl, in::mode::Hold))
            {
                auto* clipboard = context()->sys_if<sys::ext::SystemClipboard>();
                if (clipboard)
                {
                    if (frame.active(in::key('a'), in::mode::Press))
                    {
                        highlight_start_ = 0;
                        highlight_end_ = data::text.size();
                        write = true;
                    }
                    else if (
                        frame.active(in::key('v'), in::mode::Press) &&
                        !(data::input_options & ReadOnly)
                    )
                    {
                        if (!clipboard->empty())
                        {
                            _remove_highlighted();
                            const auto v = clipboard->paste();
                            data::text.insert(ptr_, v);
                            _move_right(v.size());
                            flag |= WriteType::Offset;
                            write = true;
                        }
                    }
                    else if (
                        frame.active(in::key('x'), in::mode::Press) &&
                        highlight_start_ != highlight_end_ && !(data::input_options & ReadOnly)
                    )
                    {
                        clipboard->copy(_highlighted_value());
                        _remove_highlighted();
                    }
                    else if (frame.active(in::key('c'), in::mode::Press) && _is_highlighted())
                    {
                        clipboard->copy(_highlighted_value());
                    }
                }
            }
        }
        if (frame.had_event(in::Event::KeySequenceInput) && !(data::input_options & ReadOnly))
        {
            if (_is_highlighted())
                _remove_highlighted();
            if (ptr_ > data::text.size())
                ptr_ = data::text.size();

            auto seq = std::string(frame.sequence());
            if (frame.active(in::special::Shift, in::mode::Hold))
            {
                std::transform(
                    seq.cbegin(),
                    seq.cend(),
                    seq.begin(),
                    [](char ch) -> char { return std::toupper(ch); }
                );
            }

            data::text.insert(data::text.begin() + ptr_, seq.begin(), seq.end());
            _move_right(seq.size());
            flag |= WriteType::Offset;
            write = true;
        }

        if (write)
        {
            _signal_write(flag);
        }
    }
    void Input::_frame_impl(PixelBuffer::View buffer)
    {
        const auto [width, height] = box();
        const auto color = Pixel::combine(data::foreground_color, data::background_color);

        // Draw the pre-value
        {
            buffer.fill(data::background_color);
            for (std::size_t i = 0; i < std::min(width, int(data::pre.size())); i++)
            {
                auto& p = buffer.at(i);
                p = color;
                p.v = data::pre[i];
            }
        }

        // Correct bounds
        {
            if (ptr_ > data::text.size())
                ptr_ = data::text.size();
            if (bound_lower_ > data::text.size())
                bound_lower_ = data::text.size();
        }

        // Draw the input
        {
            const auto draw_ptr =
                bool(data::input_options & InputOptions::DrawPtr) && getstate(Focused);
            if (!data::text.empty())
            {
                const auto hlower = std::min(highlight_start_, highlight_end_);
                const auto hupper = std::max(highlight_start_, highlight_end_);
                const auto m = std::min(bound_lower_ + width, int(data::text.size())) + draw_ptr;

                for (std::size_t pi = data::pre.size(), i = bound_lower_, d = 0;
                     i < m && pi < buffer.width();
                     i++, pi++)
                {
                    if (draw_ptr && i == ptr_)
                    {
                        d = 1;
                    }
                    else
                    {
                        auto& px = buffer.at(pi);
                        if (i - d >= hlower && i - d < hupper)
                        {
                            px.blend(color.mask(data::marked_mask));
                        }
                        else
                            px.blend(color);

                        if (data::input_options & InputOptions::Censor)
                        {
                            px.v = '*';
                        }
                        else
                        {
                            px.v = data::text[i - d];
                        }
                    }
                }
            }
            else if (!data::hint.empty() && !draw_ptr)
            {
                const auto hcolor = data::foreground_color.alpha(0.5f);
                for (std::size_t i = pre.size();
                     i < std::min<std::size_t>(pre.size() + hint.size(), width);
                     i++)
                {
                    buffer.at(i).blend(hcolor.stylize(d2::px::foreground::Style::Bold)
                                           .extend(data::hint[i - pre.size()]));
                }
            }

            if (const auto off = ptr_ + pre.size() - bound_lower_; draw_ptr && off < buffer.width())
            {
                _write_ptr(buffer.at(off));
            }
        }
    }

    Input::~Input()
    {
        if (data::input_options & data::Censor)
            std::fill(data::text.begin(), data::text.end(), 0);
    }

    void Input::sync()
    {
        _move_right(std::numeric_limits<std::size_t>::max());
    }
    void Input::jump(std::size_t pos)
    {
        if (pos < ptr_)
        {
            _move_left(ptr_ - pos);
        }
        else if (pos > ptr_)
        {
            _move_right(pos - ptr_);
        }
    }
} // namespace d2::dx
