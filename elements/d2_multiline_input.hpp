#ifndef D2_MULTILINE_INPUT_HPP
#define D2_MULTILINE_INPUT_HPP

#include <d2_fragmented_buffer.hpp>
#include <d2_styles.hpp>
#include <d2_tree_element.hpp>
#include <d2_tree_element_frwd.hpp>
#include <d2_tree_parent.hpp>
#include <elements/d2_input.hpp>
#include <elements/d2_slider.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace d2
{
    namespace style
    {
        // clang-format off
        D2_UAI_INTERFACE(MultiInput,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                bool enable_scrollbar_horizontal{ false };
                bool enable_scrollbar_vertical{ true };
            ),
            D2_UAI_PROPS(
                EnableScrollVert,
                EnableScrollHoriz
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(EnableScrollVert, enable_scrollbar_vertical, Masked)
                D2_UAI_PROP(EnableScrollHoriz, enable_scrollbar_horizontal, Masked)
            )
        )
        // clang-format on
    } // namespace style

    namespace dx
    {
        class MultiInput : public style::UAIE<
                               MetaParentElement,
                               MultiInput,
                               style::ILayout,
                               style::IColors,
                               style::IMultiInput,
                               style::IInput,
                               style::IResponsive<Input, string>::type
                           >
        {
        public:
            using data = style::UAIE<
                MetaParentElement,
                MultiInput,
                style::ILayout,
                style::IColors,
                style::IMultiInput,
                style::IInput,
                style::IResponsive<Input, string>::type
            >;
            using data::data;
        protected:
            struct DocModel
            {
                std::vector<std::size_t> starts{0};
                std::vector<std::size_t> line_lengths{0};
                std::size_t length{0};
                std::size_t max_width{0};
            };
            struct Viewport
            {
                std::size_t width{0};
                std::size_t height{0};
                bool show_hscroll{false};
                bool show_vscroll{false};
            };
        protected:
            IOContext::future<void> _btask{};
            fb::FragmentedBuffer _buffer{};
            mutable DocModel _dm_cache{};

            std::shared_ptr<VerticalSlider> _vscrollbar{nullptr};
            std::shared_ptr<Slider> _hscrollbar{nullptr};

            bool _syncing_vscroll_{false};
            bool _syncing_hscroll_{false};

            std::size_t _ptr_abs{0};
            std::size_t _preferred_x{0};

            std::size_t _view_x{0};
            std::size_t _view_y{0};

            std::size_t _highlight_start{0};
            std::size_t _highlight_end{0};

            bool _ptr_blink{false};
            mutable bool _dirty_dm{true};
        protected:
            void _write_ptr(Pixel& px);

            void _doc_dirty() const;
            const DocModel& _doc_model() const;
            std::size_t _line_count(const DocModel& doc) const;
            std::size_t _line_length(const DocModel& doc, std::size_t line) const;
            std::pair<std::size_t, std::size_t>
            _line_col_from_abs(const DocModel& doc, std::size_t abs) const;
            std::size_t
            _abs_from_line_col(const DocModel& doc, std::size_t line, std::size_t col) const;
            string _range_value(std::size_t pos, std::size_t len) const;
            string _line_segment(
                const DocModel& doc, std::size_t line, std::size_t col, std::size_t len
            ) const;

            bool _ptr_overlapped() const;
            bool _draw_ptr() const;
            std::size_t _render_max_width(const DocModel& doc) const;

            bool _fixed_width() const;
            bool _fixed_height() const;
            int _vscroll_width() const;
            int _hscroll_height() const;
            Viewport _viewport(const DocModel& doc) const;

            void _sync_scrollbars();
            void _scroll_x_to(std::size_t off);
            void _scroll_y_to(std::size_t off);

            void _clamp_ptr();
            void _sync_ptr();
            void _ensure_ptr_visible();

            bool _is_highlighted() const;
            void _remove_highlighted();
            void _begin_highlight_if();
            void _reset_highlight();
            string _highlighted_value() const;

            void _start_blink();
            void _stop_blink();
            void _freeze_blink();
            void _unfreeze_blink();

            std::size_t _mouse_abs_pos() const;

            void _move_to(std::size_t pos, bool keep_highlight = false);
            void _move_right(std::size_t cnt = 1);
            void _move_left(std::size_t cnt = 1);
            void _move_up(std::size_t cnt = 1);
            void _move_down(std::size_t cnt = 1);

            void _insert(std::string_view value);
            void _erase_before(std::size_t len = 1);
            void _erase_after(std::size_t len = 1);

            virtual void
            _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
            virtual Unit _layout_impl(Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(in::InputFrame& frame) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
            virtual void _update_layout_impl() override;
            virtual void foreach_internal(foreach_internal_callback callback) const override;
        public:
            string value() const;
            void value(std::string text);

            void clear();

            void undo();
            void redo();

            bool is_synced() const noexcept;
            void sync();

            void jump(std::size_t pos);
            void insert(std::string_view text);
            void append(std::string_view text);
            void erase(std::size_t len = 1);
            void erase_at(std::size_t pos, std::size_t len = 1);

            void disable_history();
            void enable_history();

            std::size_t cursor() const;

            d2::TreeIter<VerticalSlider> vertical_scollbar() const noexcept;
            d2::TreeIter<Slider> horizontal_scrollbar() const noexcept;

            const fb::FragmentedBuffer& buffer() const noexcept;
        };
    } // namespace dx
} // namespace d2

#endif // D2_MULTILINE_INPUT_HPP
