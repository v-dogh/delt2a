#include "d2_matrix.hpp"

namespace d2::dx
{
	Matrix::unit_meta_flag Matrix::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	char Matrix::_index_impl() const noexcept
	{
		return data::zindex;
	}

	Matrix::BoundingBox Matrix::_dimensions_impl() const noexcept
	{
		if (!data::model)
			return { 0, 0 };
		return {
			static_cast<int>(data::model->width()),
			static_cast<int>(data::model->height())
		};
	}
	Matrix::Position Matrix::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	Matrix::BoundingBox Matrix::_reserve_buffer_impl() const noexcept
	{
		if (
			data::model &&
			(data::mask_options & MaskMode::ApplyBg ||
			data::mask_options & MaskMode::ApplyFg)
			)
		{
			return {
				static_cast<int>(data::model->width()),
				static_cast<int>(data::model->height())
			};
		}
		else
			return { 0, 0 };
	}
	PixelBuffer::View Matrix::_fetch_pixel_buffer_impl() const noexcept
	{
		if (
			!data::model ||
			(data::mask_options & MaskMode::ApplyBg) ||
			(data::mask_options & MaskMode::ApplyFg)
			)
			return Element::_fetch_pixel_buffer_impl();
		return PixelBuffer::View(
			data::model.get(),
			0, 0,
			data::model->width(),
			data::model->height()
		);
	}
	bool Matrix::_provides_buffer_impl() const noexcept
	{
		return true;
	}

	void Matrix::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		if (
			data::model &&
			((data::mask_options & MaskMode::ApplyBg) ||
			(data::mask_options & MaskMode::ApplyFg))
			)
		{
			const auto vol = data::model->width() * data::model->height();
			std::size_t idx = 0;
			for (std::size_t i = 0; i < vol; i++)
			{
				if (data::mask_options & MaskMode::ApplyBg)
				{
					buffer.at(idx) = data::model->at(idx);
					if (data::mask_options & MaskMode::InterpBg)
					{
						buffer.at(idx).mask_background(data::background_mask);
					}
					else
					{
						auto& px = buffer.at(idx);
						px.r = data::background_mask.r;
						px.g = data::background_mask.g;
						px.b = data::background_mask.b;
						px.a = data::background_mask.a;
					}
				}
				if (data::mask_options & MaskMode::ApplyFg)
				{
					buffer.at(idx) = data::model->at(idx);
					if (data::mask_options & MaskMode::InterpFg)
					{
						buffer.at(idx).mask_foreground(data::foreground_mask);
					}
					else
					{
						auto& px = buffer.at(idx);
						px.rf = data::foreground_mask.r;
						px.gf = data::foreground_mask.g;
						px.bf = data::foreground_mask.b;
						px.af = data::foreground_mask.a;
					}
				}
			}
		}
	}
}
