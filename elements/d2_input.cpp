#include "elements/d2_input.hpp"
#include <core/mods/d2_ext.hpp>
#include <core/screen/d2_screen.hpp>
#include <limits>

namespace d2::dx
{
    const text::Layout& Input::_layout() const
    {
        _text.reset(data::text, data::input_options & Censor, false);
        return _text.display();
    }
    int Input::_prefix_width() const
    {
        return text::Layout(data::pre, {.newlines = false}).bounds().width;
    }
    void Input::_ensure_ptr_visible()
    {
        const auto& layout = _layout();
        ptr_ = std::min(ptr_, layout.size());
        highlight_start_ = std::min(highlight_start_, layout.size());
        highlight_end_ = std::min(highlight_end_, layout.size());
        const auto width = std::max(1, box().width - _prefix_width());
        const auto column = layout.caret(ptr_).x;
        bound_lower_ = std::clamp(bound_lower_, 0, std::max(0, layout.bounds().width + 1 - width));
        if (column < bound_lower_)
            bound_lower_ = column;
        else if (column >= bound_lower_ + width)
            bound_lower_ = column - width + 1;
    }
    std::size_t Input::_hit(Position position) const
    {
        _layout();
        position.x += bound_lower_ - _prefix_width();
        position.y = 0;
        const auto gap = getstate(Focused) && (data::input_options & DrawPtr) &&
                         !(data::input_options & PtrOverlap);
        return _text.hit(position, gap ? std::optional(ptr_) : std::nullopt);
    }
    void Input::_write_ptr(pixel& px)
    {
        const auto hidden = (data::input_options & Blink) && !ptr_blink_;
        px = pixel::combine(
            (data::input_options & AutoPtrColor) ? data::foreground_color.extend(data::ptr_color.v)
                                               : data::ptr_color,
            data::background_color);
        px.style &= ~px::style::Reserved;
        if (hidden)
            px.v = ' ';
    }
    bool Input::_is_highlighted() { return highlight_start_ != highlight_end_; }
    void Input::_erase(std::size_t begin, std::size_t end)
    {
        _layout();
        begin = std::min(begin, _text.source.size());
        end = std::clamp(end, begin, _text.source.size());
        const auto byte = _text.source.byte_offset(begin);
        data::text.erase(byte, _text.source.byte_offset(end) - byte);
        _layout();
        ptr_ = _text.source.grapheme_at(byte, text::Affinity::Downstream);
        _reset_highlight();
        _ensure_ptr_visible();
        _signal_write(WriteType::Masked);
    }
    void Input::_remove_highlighted()
    {
        if (_is_highlighted())
            _erase(std::min(highlight_start_, highlight_end_), std::max(highlight_start_, highlight_end_));
    }
    void Input::_begin_highlight_if()
    {
        if (!_is_highlighted())
            highlight_start_ = highlight_end_ = ptr_;
    }
    void Input::_reset_highlight() { highlight_start_ = highlight_end_ = ptr_; }
    void Input::_reset_ptrs()
    {
        ptr_ = bound_lower_ = 0;
        _reset_highlight();
    }
    string Input::_highlighted_value()
    {
        _ensure_ptr_visible();
        return std::string(_text.source.extract({highlight_start_, highlight_end_}));
    }
    void Input::_insert(std::string_view value)
    {
        if (value.empty())
            return;
        _ensure_ptr_visible();
        const auto begin = _is_highlighted() ? std::min(highlight_start_, highlight_end_) : ptr_;
        const auto end = _is_highlighted() ? std::max(highlight_start_, highlight_end_) : ptr_;
        const auto byte = _text.source.byte_offset(begin);
        const auto count = value.size();
        data::text.replace(byte, _text.source.byte_offset(end) - byte, value);
        _layout();
        ptr_ = _text.source.grapheme_at(byte + count, text::Affinity::Downstream);
        _reset_highlight();
        _ensure_ptr_visible();
        _signal_write(WriteType::Masked);
    }
    void Input::_start_blink()
    {
        if ((data::input_options & InputOptions::Blink) && getstate(State::Focused) &&
            btask_ == nullptr)
        {
            btask_ = context()->scheduler()->launch_cyclic(
                std::chrono::milliseconds(500),
                [weak = weak_from_this()](mt::future<void> task)
                {
                    if (auto element = weak.lock())
                    {
                        auto& self = static_cast<Input&>(*element);
                        self.ptr_blink_ = !self.ptr_blink_;
                        self._signal_write(WriteType::Style);
                    }
                    else
                        task.discard();
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
        if (getstate(State::Focused) && btask_ == nullptr)
            _start_blink();
    }


    void Input::_move_to(std::size_t pos, bool select)
    {
        _ensure_ptr_visible();
        if (select)
            _begin_highlight_if();
        ptr_ = std::min(pos, _layout().size());
        if (select)
            highlight_end_ = ptr_;
        else
            _reset_highlight();
        _ensure_ptr_visible();
        _signal_write(WriteType::Style);
    }
    void Input::_move_right(std::size_t cnt)
    {
        _ensure_ptr_visible();
        _move_to(ptr_ + std::min(cnt, _layout().size() - ptr_));
    }
    void Input::_move_left(std::size_t cnt) { _move_to(ptr_ - std::min(ptr_, cnt)); }
    bool Input::_provides_input_impl() const { return true; }
    Unit Input::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X: return data::x;
        case Element::Layout::Y: return data::y;
        case Element::Layout::Width:
            if (data::width.getunits() == Unit::Auto)
                return Unit(_prefix_width() + std::max(_layout().bounds().width +
                    int(getstate(Focused) && (data::input_options & DrawPtr)),
                    text::Layout(data::hint, {.newlines = false}).bounds().width) + data::width.raw());
            return data::width;
        case Element::Layout::Height:
            if (data::height.getunits() == Unit::Auto)
                return Unit(1 + data::height.raw());
            return data::height;
        }
        return data::width;
    }
    void Input::_state_change_impl(State state, bool value)
    {
        if (state == State::Focused)
        {
            if (value)
                _start_blink();
            else
            {
                _stop_blink();
                _reset_highlight();
            }
            _signal_write(WriteType::Dimensions);
        }
        else if (state == State::Clicked && value)
        {
            _ensure_ptr_visible();
            _move_to(_hit(mouse_object_space()));
            _begin_highlight_if();
        }
        else if (state == State::Swapped)
        {
            if (value)
                _freeze_blink();
            else
                _unfreeze_blink();
        }
    }
    void Input::_event_impl(in::InputFrame& frame)
    {
        _ensure_ptr_visible();
        const auto keys = frame.map().current;
        const auto shift = keys.test(std::size_t(in::special::Shift));
        const auto ctrl = keys.test(std::size_t(in::special::LeftControl));
        const auto readonly = bool(data::input_options & ReadOnly);
        if ((frame.had_event(in::Event::MouseMovement) || frame.had_event(in::Event::KeyMouseInput)) &&
            frame.active(in::mouse::Left, in::mode::Hold))
            _move_to(_hit(mouse_object_space()), true);
        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::Enter, in::mode::Hold) && !readonly)
            {
                if (!data::text.empty())
                {
                    auto submitted = std::move(data::text);
                    data::text.clear();
                    _reset_ptrs();
                    _signal_write(WriteType::Masked);
                    if (data::on_submit)
                        data::on_submit(std::static_pointer_cast<Input>(shared_from_this()), std::move(submitted));
                }
            }
            else if (frame.active(in::special::Backspace, in::mode::Hold) && !readonly)
            {
                if (_is_highlighted()) _remove_highlighted();
                else if (ptr_) _erase(ptr_ - 1, ptr_);
            }
            else if (frame.active(in::special::Delete, in::mode::Hold) && !readonly)
            {
                if (_is_highlighted()) _remove_highlighted();
                else if (ptr_ < _layout().size()) _erase(ptr_, ptr_ + 1);
            }
            else if (frame.active(in::special::ArrowLeft, in::mode::Hold))
                _move_to(ptr_ ? ptr_ - 1 : 0, shift);
            else if (frame.active(in::special::ArrowRight, in::mode::Hold))
                _move_to(ptr_ + (ptr_ < _layout().size()), shift);
            else if (frame.active(in::special::Home, in::mode::Hold))
                _move_to(0, shift);
            else if (frame.active(in::special::End, in::mode::Hold))
                _move_to(_layout().size(), shift);
            else if (frame.active(in::special::Escape, in::mode::Hold))
                screen()->focus(nullptr);
            else if (ctrl)
            {
                if (frame.active(in::key('a'), in::mode::Press))
                {
                    _move_to(0);
                    _move_to(_layout().size(), true);
                }
                else if (auto ctx = state() ? context() : nullptr; ctx)
                {
                    if (auto clipboard = ctx->sys_if<sys::SystemClipboard>(); clipboard)
                    {
                        if (frame.active(in::key('v'), in::mode::Press) && !readonly)
                            _insert(clipboard->paste());
                        else if (frame.active(in::key('x'), in::mode::Press) && !readonly && _is_highlighted())
                        {
                            clipboard->copy(_highlighted_value());
                            _remove_highlighted();
                        }
                        else if (frame.active(in::key('c'), in::mode::Press) && _is_highlighted())
                            clipboard->copy(_highlighted_value());
                    }
                }
            }
        }
        if (frame.had_event(in::Event::KeySequenceInput) && !readonly && !ctrl)
            _insert(frame.sequence());
    }
    void Input::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(data::background_color);
        if (buffer.empty())
            return;
        _ensure_ptr_visible();
        const auto color = pixel::combine(data::foreground_color, data::background_color);
        const auto prefix = _prefix_width();
        text::Layout(data::pre, {.newlines = false}).draw(buffer, color);
        const auto width = std::max(0, buffer.width() - prefix);
        if (!width)
            return;
        auto content = PixelBuffer::View(buffer, prefix, 0, width, 1);
        const auto draw_ptr = (data::input_options & DrawPtr) && getstate(Focused);
        const auto gap = draw_ptr && !(data::input_options & PtrOverlap);
        _layout().draw(content, color, {-bound_lower_, 0}, text::Range{highlight_start_, highlight_end_},
                       data::highlight_mask, gap ? std::optional(ptr_) : std::nullopt);
        if (data::text.empty() && !draw_ptr)
            text::Layout(data::hint, {.newlines = false}).draw(content,
                pixel::combine(data::foreground_color.alpha(0.5f).stylize(px::style::Bold), data::background_color));
        const auto column = _layout().caret(ptr_).x - bound_lower_;
        if (draw_ptr && column >= 0 && column < width)
        {
            if (!gap && ptr_ < _layout().size())
                for (int i = 0; i < _layout().glyphs()[ptr_].columns && column + i < width; ++i)
                    content.at(column + i, 0) = pixel::combine(data::foreground_color.extend(' '), data::background_color);
            _write_ptr(content.at(column, 0));
        }
    }
    Input::~Input()
    {
        btask_.discard();
        if (data::input_options & Censor)
            std::fill(data::text.begin(), data::text.end(), 0);
    }
    void Input::sync() { _move_to(_layout().size()); }
    void Input::jump(std::size_t pos) { _move_to(pos); }
    std::size_t Input::cursor() const { return std::min(ptr_, _layout().size()); }
} // namespace d2::dx
