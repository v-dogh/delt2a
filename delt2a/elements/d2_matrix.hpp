#ifndef D2_MATRIX_HPP
#define D2_MATRIX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace dx
	{
		class MatrixModel : public PixelBuffer
		{
		public:
			enum class ModelType
			{
				Raw,
				Visual
			};
			enum ModelHeader : std::uint8_t
			{
				Endian = 1 << 0,
				Compressed = 1 << 1,
				Extended = 1 << 2,
			};

			using ptr = std::shared_ptr<MatrixModel>;
			using PixelBuffer::PixelBuffer;
		public:
			static auto make(int width = 0, int height = 0) noexcept;

			void load_model(const std::string& path, ModelType type);
			void save_model(const std::string& path, ModelType type);
		};
	}
	namespace style
	{
		struct Model
		{
			enum MaskMode
			{
				ApplyFg = 1 << 0,
				ApplyBg = 1 << 1,
				InterpFg = 1 << 2,
				InterpBg = 1 << 3,
			};

			dx::MatrixModel::ptr model{ nullptr };
			PixelBackground background_mask{};
			PixelForeground foreground_mask{};
			unsigned char mask_options{ 0x00 };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Masked)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, model)
				D2_UAI_GET_VAR_A(1, background_mask)
				D2_UAI_GET_VAR_A(2, foreground_mask)
				D2_UAI_GET_VAR_A(3, mask_options)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IModel : Model, InterfaceHelper<IModel, PropBase, 4>
		{
			using data = Model;
			enum Property : uai_property
			{
				Model = PropBase,
				BackgroundMask,
				ForegroundMask,
				MaskOptions
			};
		};
		using IZModel = IModel<0>;
	}
	namespace dx
	{
		class Matrix :
			public style::UAI<Matrix, style::ILayout, style::IModel>,
			public impl::UnitUpdateHelper<Matrix>
		{
		public:
			friend class UnitUpdateHelper;
			using data = style::UAI<Matrix, style::ILayout, style::IModel>;
			using data::data;
		protected:
			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual char _index_impl() const noexcept override;

			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;

			virtual BoundingBox _reserve_buffer_impl() const noexcept override;
			virtual PixelBuffer::View _fetch_pixel_buffer_impl() const noexcept override;
			virtual bool _provides_buffer_impl() const noexcept override;

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
		};
	}
}

#endif // D2_MATRIX_HPP
