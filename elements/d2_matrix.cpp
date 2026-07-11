#include "elements/d2_matrix.hpp"

namespace d2::dx
{
    Unit Matrix::_layout_impl(enum Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X:
            return data::x;
        case Element::Layout::Y:
            return data::y;
        case Element::Layout::Width:
            if (!data::model)
                return HDUnit(0);
            return data::model->width();
        case Element::Layout::Height:
            if (!data::model)
                return VDUnit(0);
            return data::model->height();
        }
    }
    BoundingBox Matrix::_reserve_buffer_impl() const
    {
        if (data::model &&
            (data::mask_options & MaskMode::ApplyBg || data::mask_options & MaskMode::ApplyFg))
        {
            return {
                static_cast<int>(data::model->width()), static_cast<int>(data::model->height())
            };
        }
        else
            return {0, 0};
    }
    PixelBuffer::View Matrix::_fetch_pixel_buffer_impl() const
    {
        if (!data::model || (data::mask_options & MaskMode::ApplyBg) ||
            (data::mask_options & MaskMode::ApplyFg))
            return Element::_fetch_pixel_buffer_impl();
        return PixelBuffer::View(
            data::model.get(), 0, 0, data::model->width(), data::model->height()
        );
    }
    bool Matrix::_provides_buffer_impl() const
    {
        return true;
    }

    void Matrix::_frame_impl(PixelBuffer::View buffer)
    {
        if (data::model &&
            ((data::mask_options & MaskMode::ApplyBg) || (data::mask_options & MaskMode::ApplyFg)))
        {
            const auto vol = data::model->width() * data::model->height();
            std::size_t idx = 0;
            for (std::size_t i = 0; i < vol; i++)
            {
                auto& px = buffer.at(idx);
                if (data::mask_options & MaskMode::ApplyBg)
                {
                    px = data::model->at(idx);
                    if (data::mask_options & MaskMode::InterpBg)
                    {
                        px = px.mask_background(data::background_mask);
                    }
                    else
                    {
                        px.r = data::background_mask.r;
                        px.g = data::background_mask.g;
                        px.b = data::background_mask.b;
                        px.a = data::background_mask.a;
                    }
                }
                if (data::mask_options & MaskMode::ApplyFg)
                {
                    px = data::model->at(idx);
                    if (data::mask_options & MaskMode::InterpFg)
                    {
                        px = px.mask_foreground(data::foreground_mask);
                    }
                    else
                    {
                        px.rf = data::foreground_mask.r;
                        px.gf = data::foreground_mask.g;
                        px.bf = data::foreground_mask.b;
                        px.af = data::foreground_mask.a;
                    }
                }
            }
        }
    }
} // namespace d2::dx
