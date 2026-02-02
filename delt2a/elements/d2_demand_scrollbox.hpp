#ifndef D2_DEMAND_SCROLLBOX_HPP
#define D2_DEMAND_SCROLLBOX_HPP

#include "d2_slider.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace dx
    {
        class DemandScrollBox;
        class EntrySeed
        {
        private:
            std::size_t _idx{ 0 };
            d2::TreeIter<DemandScrollBox> _ptr{};
            d2::Element::ptr _element{ nullptr };
            bool _created{ false };

            d2::Element::ptr _get() const noexcept { return _element; }
        public:
            friend class DemandScrollBox;

            EntrySeed(const EntrySeed&) = delete;
            EntrySeed(EntrySeed&&) = delete;
            EntrySeed(std::size_t idx, d2::TreeIter<DemandScrollBox> ptr) :
                _idx(idx), _ptr(ptr) {}

            template<typename Element, typename... Argv>
            d2::TreeIter<Element> create(Argv&&... args)
            {
                if (_created)
                    throw std::logic_error{ "Seed can only be used once" };
                _created = true;
                auto ptr = d2::Element::make<Element>(
                    "",
                    _ptr.asp()->state(),
                    _ptr.asp(),
                    std::forward<Argv>(args)...
                );
                ptr->initialize();
                ptr->setstate(Element::State::Created, true);
                ptr->setstate(Element::State::Swapped, _ptr.asp()
                    ->getstate(Element::Swapped));
                _element = ptr;
                return ptr;
            }

            std::size_t index() const noexcept { return _idx; }
            d2::TreeIter<DemandScrollBox> parent() const noexcept { return _ptr; }

            EntrySeed& operator=(const EntrySeed&) = delete;
            EntrySeed& operator=(EntrySeed&&) = delete;
        };
    }
    namespace style
    {
        D2_UAI_INTERFACE(DemandScrollBox,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                std::size_t preload{ 2 };
                std::size_t length{ 0 };
                d2::VDUnit static_height_hint{ 0 };
                d2::HDUnit padding_horizontal{ 0.0_px };
                d2::VDUnit padding_vertical{ 0.0_px };
                bool swap_out{ true };
                bool hide_slider{ false };
                // Returns the new element at the passed index
                std::function<void(dx::EntrySeed&)> fetch_callback{ nullptr };
            ),
            D2_UAI_PROPS(
                Preload,
                Length,
                StaticHeightHint,
                PaddingHorizontal,
                PaddingVertical,
                SwapOut,
                HideSlider,
                FetchCallback
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Preload, preload, 0x00)
                D2_UAI_PROP(Length, length, Style)
                D2_UAI_PROP(StaticHeightHint, static_height_hint, Style)
                D2_UAI_PROP(PaddingHorizontal, padding_horizontal, Style)
                D2_UAI_PROP(PaddingVertical, padding_vertical, Style)
                D2_UAI_PROP(SwapOut, swap_out, 0x00)
                D2_UAI_PROP(HideSlider, hide_slider, Style)
                D2_UAI_PROP(FetchCallback, fetch_callback, 0x00)
            )
        )
    }
    namespace dx
    {
        class DemandScrollBox  :
            public style::UAIE<
                MetaParentElement,
                DemandScrollBox,
                style::ILayout,
                style::IContainer,
                style::IColors,
                style::IDemandScrollBox
            >,
            public impl::ContainerHelper<DemandScrollBox>
        {
        public:
            friend class ContainerHelper<DemandScrollBox>;
            using seed = EntrySeed;
            using data = style::UAIE<
                MetaParentElement,
                DemandScrollBox,
                style::ILayout,
                style::IContainer,
                style::IColors,
                style::IDemandScrollBox
            >;
            using data::data;
        protected:
            std::size_t _top_index = 0;
            int _top_inset = 0;
            int _view_base = 0;
            int _zero_start = 0;

            std::vector<int> _height_cache;
            std::size_t _avg_height_cnt = 0;
            double _avg_height = 0.0;

            std::shared_ptr<VerticalSlider> _scrollbar{ nullptr };
            std::vector<d2::Element::ptr> _view{};
            int _prev_offset{ 0 };
            int _offset{ 0 };

            void _update_view();

            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(sys::screen::Event ev) override;

            virtual int _get_border_impl(BorderType type, cptr elem) const override;
            virtual Unit _layout_impl(enum Element::Layout type) const override;
            virtual void _layout_for_impl(enum Element::Layout type, cptr ptr) const override;

            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            TreeIter<VerticalSlider> scrollbar() const;
            void scroll_to(int offset);

            virtual void foreach(foreach_callback callback) const noexcept override;
            virtual void foreach_internal(foreach_internal_callback callback) const override;
        };
    }
}

#endif // D2_DEMAND_SCROLLBOX_HPP
