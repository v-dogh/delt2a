// #include "d2_multiline_input.hpp"
// #include "../d2_screen.hpp"

// namespace d2::dx
// {
//     void MultiInput::_move_right(std::size_t cnt)
//     {
//         buffer.ptr.right(cnt);
//         _signal_write(WriteType::Style);
//     }
//     void MultiInput::_move_left(std::size_t cnt)
//     {
//         buffer.ptr.left(cnt);
//         _signal_write(WriteType::Style);
//     }

//     bool MultiInput::_provides_input_impl() const
//     {
//         return true;
//     }
//     Unit MultiInput::_layout_impl(Element::Layout type) const
//     {
//         switch (type)
//         {
//         case Element::Layout::X: return data::x;
//         case Element::Layout::Y: return data::y;
//         case Element::Layout::Width: return data::width;
//         case Element::Layout::Height: return data::height;
//         }
//     }
//     void MultiInput::_state_change_impl(State state, bool value)
//     {
//         if (state == State::Focused)
//         {
//             // if (value)
//             // {
//             //     _start_blink();
//             // }
//             // else
//             // {
//             //     _stop_blink();
//             //     _reset_highlight();
//             // }
//         }
//         else if (state == State::Clicked && value)
//         {
//             // const auto [ x, y ] = mouse_object_space();
//             // if (x >= data::pre.size())
//             //     ptr_ = _bound_lower + (x - data::pre.size());
//             // _reset_highlight();
//             // _begin_highlight_if();
//         }
//         else if (state == State::Swapped)
//         {
//             if (value)
//             {
//                 // _freeze_blink();
//             }
//             else
//             {
//                 // _unfreeze_blink();
//             }
//         }
//     }
//     void MultiInput::_event_impl(IOContext::Event ev)
//     {
//         write_flag flag = WriteType::Style;
//         bool write = false;

//         const auto input = context()->input();
//         if (ev == IOContext::Event::MouseInput)
//         {
//             if (input->is_pressed_mouse(sys::input::Left))
//             {
//                 const auto [ x, y ] = mouse_object_space();
//                 if (x >= data::pre.size())
//                 {
//                     _highlight_end = _bound_lower + (x - data::pre.size());
//                     write = true;
//                 }
//             }
//         }
//         else if (ev == IOContext::Event::KeyInput)
//         {
//             if (input->is_pressed(sys::input::Enter))
//             {
//                 if (!data::text.empty())
//                 {
//                     if (data::on_submit != nullptr)
//                         data::on_submit(shared_from_this(), std::move(data::text));
//                     data::text.clear();
//                     _reset_ptrs();
//                     flag |= WriteType::Offset;
//                     write = true;
//                 }
//             }
//             else if (input->is_pressed(sys::input::Backspace))
//             {
//                 if (_is_highlighted())
//                 {
//                     _remove_highlighted();
//                 }
//                 else if (!data::text.empty() && ptr_)
//                 {
//                     data::text.erase(ptr_ - 1, 1);
//                     if (ptr_ == _bound_lower)
//                         _bound_lower--;
//                     ptr_--;
//                     write = true;
//                 }
//                 flag |= WriteType::Offset;
//             }
//             else if (input->is_pressed(sys::input::Delete))
//             {
//                 if (_is_highlighted())
//                 {
//                     _remove_highlighted();
//                 }
//                 else if (ptr_ < data::text.size())
//                 {
//                     data::text.erase(ptr_, 1);
//                     write = true;
//                 }
//                 flag |= WriteType::Offset;
//             }
//             else if (input->is_pressed(sys::input::ArrowLeft))
//             {
//                 if (input->is_pressed(sys::input::Shift))
//                 {
//                     _begin_highlight_if();
//                     if (_highlight_end > 0)
//                     {
//                         _highlight_end--;
//                         write = true;
//                     }
//                 }
//                 else
//                     _move_left();
//             }
//             else if (input->is_pressed(sys::input::ArrowRight))
//             {
//                 if (input->is_pressed(sys::input::Shift))
//                 {
//                     _begin_highlight_if();
//                     if (_highlight_end < data::text.size())
//                     {
//                         _highlight_end++;
//                         if (_highlight_end > _bound_lower + box().width)
//                             _bound_lower++;
//                         write = true;
//                     }
//                 }
//                 else
//                     _move_right();
//             }
//             else if (input->is_pressed(sys::input::Escape))
//             {
//                 screen()->focus(nullptr);
//             }
//             else if (input->is_pressed(sys::input::LeftControl))
//             {
//                 auto* clipboard = context()->sys_if<sys::ext::SystemClipboard>();
//                 if (clipboard)
//                 {
//                     if (input->is_pressed(sys::input::key('a')))
//                     {
//                         _highlight_start = 0;
//                         _highlight_end = data::text.size();
//                         write = true;
//                     }
//                     else if (input->is_pressed(sys::input::key('v')))
//                     {
//                         if (!clipboard->empty())
//                         {
//                             _remove_highlighted();
//                             const auto v = clipboard->paste();
//                             data::text.insert(ptr_, v);
//                             _move_right(v.size());
//                             flag |= WriteType::Offset;
//                             write = true;
//                         }
//                     }
//                     else if (
//                         input->is_pressed(sys::input::key('x')) &&
//                         _highlight_start != _highlight_end
//                         )
//                     {
//                         clipboard->copy(_highlighted_value());
//                         _remove_highlighted();
//                     }
//                     else if (
//                         input->is_pressed(sys::input::key('c')) &&
//                         _is_highlighted()
//                         )
//                     {
//                         clipboard->copy(_highlighted_value());
//                     }
//                 }
//             }
//         }
//         else if (ev == IOContext::Event::KeySequenceInput)
//         {
//             if (_is_highlighted())
//                 _remove_highlighted();
//             if (ptr_ > data::text.size())
//                 ptr_ = data::text.size();

//             auto seq = input->key_sequence();
//             if (input->is_pressed(sys::input::Shift))
//             {
//                 std::transform(seq.cbegin(), seq.cend(), seq.begin(), [](char ch) -> char
//                                {
//                                    return std::toupper(ch);
//                                });
//             }

//             data::text.insert(data::text.begin() + ptr_, seq.begin(), seq.end());
//             _move_right(seq.size());
//             flag |= WriteType::Offset;
//             write = true;
//         }

//         if (write)
//         {
//             _signal_write(flag);
//         }
//     }
//     void MultiInput::_frame_impl(PixelBuffer::View buffer)
//     {
//         const auto [ width, height ] = box();
//         const auto color = Pixel::combine(
//             data::foreground_color,
//             data::background_color
//         );

//         // Draw the pre-value
//         {
//             buffer.fill(data::background_color);
//             for (std::size_t i = 0; i < height; i++)
//                 for (std::size_t j = 0; j < std::min(width, int(data::pre.size())); j++)
//                 {
//                     auto& p = buffer.at(i, j);
//                     p = color;
//                     p.v = data::pre[i];
//                 }
//         }

//         // Draw the input
//         {
//             const auto hlower = std::min(_highlight_start, _highlight_end);
//             const auto hupper = std::max(_highlight_start, _highlight_end);
//             const auto m =
//                 std::min(_bound_lower + width, int(data::text.size())) +
//                 bool(data::input_options & InputOptions::DrawPtr);

//             for (std::size_t pi = data::pre.size(),
//                  i = _bound_lower, d = 0;
//                  i < m && pi < buffer.width();
//                  i++, pi++)
//             {
//                 if ((data::input_options & InputOptions::DrawPtr) &&
//                     i == ptr_)
//                 {
//                     d = 1;
//                 }
//                 else
//                 {
//                     auto& px = buffer.at(pi);
//                     if (i - d >= hlower && i - d < hupper)
//                     {
//                         px = color.mask(
//                             data::marked_mask
//                             );
//                     }
//                     else
//                         px = color;

//                     if (data::input_options & InputOptions::Censor)
//                     {
//                         px.v = '*';
//                     }
//                     else
//                     {
//                         px.v = data::text[i - d];
//                     }
//                 }
//             }
//         }
//     }

//     MultiInput::~MultiInput()
//     {
//         // if (data::input_options & data::Censor)
//         //     std::fill(data::text.begin(), data::text.end(), 0);
//     }
// }
