#ifndef D2_MATRIX_HPP
#define D2_MATRIX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"
#include <filesystem>
#include <fstream>

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
				// If set the byte order is little endian
				Endian = 1 << 0,
				Compressed = 1 << 1,
				// Uses UTF-8 code points
				Extended = 1 << 2,
			};

			using ptr = std::shared_ptr<MatrixModel>;
			using PixelBuffer::PixelBuffer;
		public:
			static auto make(int width = 0, int height = 0) noexcept
			{
				auto ptr = std::make_shared<MatrixModel>();
				ptr->set_size(width, height);
				return ptr;
			}

			void load_model(const std::string& path, ModelType type)
			{
				std::fstream file(path, std::ios::in | std::ios::binary);
				std::string data;
				std::vector<Pixel> buffer{};
				std::size_t width = 0;
				std::size_t height = 0;
				data.resize(std::filesystem::file_size(path));
				file.read(data.data(), data.size());

				if (type == ModelType::Raw)
				{
					if (data.size() < sizeof(std::uint32_t) * 2 + 1)
						throw std::runtime_error{ "Attempt to load invalid model" };

					const auto header = *reinterpret_cast<const std::uint8_t*>(data.data());
					const bool endian = header & Endian;
					auto byteswap_arithmetic_if = [endian](auto value)
					{
						if (endian && std::endian::native != std::endian::little)
						{
							auto* ptr = reinterpret_cast<unsigned char*>(value);
							for (std::size_t i = 0; i < sizeof(value) / 2; i++)
							{
								const auto off = sizeof(value) - i - 1;
								std::swap(ptr[i], ptr[off]);
							}
						}
						return value;
					};

					auto off = 1;
					width = byteswap_arithmetic_if(
						*reinterpret_cast<const std::uint32_t*>(data.data() + off)
					);
					off += sizeof(std::uint32_t);
					height = byteswap_arithmetic_if(
						*reinterpret_cast<const std::uint32_t*>(data.data() + off)
					);
					off += sizeof(std::uint32_t);

					if (data.size() < width * height + sizeof(std::uint32_t) * 2 + 1)
						throw std::runtime_error{ "Attempt to load invalid model; size mismatch" };

					if (header & Compressed)
					{
						throw std::logic_error{ "Compression is not implemented yet" };
					}
					else
					{
						buffer.reserve(width * height);
						for (std::size_t i = off; i < data.size(); i += sizeof(Pixel))
						{
							buffer.push_back(
								*reinterpret_cast<const Pixel*>(&data[i])
							);
						}
					}
				}
				else if (type == ModelType::Visual)
				{
					// Find line width
					int cwidth = 0;
					for (std::size_t i = 0;; i++, cwidth++)
					{
						const bool e = i >= data.size();
						if (data[i] == '\n' || e)
						{
							width = std::max(width, std::size_t(cwidth));
							cwidth = -1;
							height++;
							if (e) break;
						}
					}

					buffer.reserve(width * height);

					std::size_t idx = 0;
					while (idx < data.size())
					{
						std::size_t lwidth = 0;
						for (; idx < data.size() && data[idx] != '\n'; idx++, lwidth++)
						{
							buffer.push_back(Pixel{
								.v = data[idx]
							});
						}

						lwidth = width - lwidth;
						while (lwidth)
						{
							buffer.push_back(Pixel{
								.v = ' '
							});
							lwidth--;
						}

						idx++;
					}
				}

				reset(std::move(buffer), width, height);
			}
			void save_model(const std::string& path, ModelType type)
			{
				std::fstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);

				if (type == ModelType::Raw)
				{
					const std::uint8_t header =
						(std::endian::native == std::endian::little) * Endian;
					const std::uint32_t width = width_;
					const std::uint32_t height = height_;
					file << header;

					file.write(
						reinterpret_cast<const char*>(&width),
						sizeof(width)
					);
					file.write(
						reinterpret_cast<const char*>(&height),
						sizeof(height)
					);
					file.write(
						reinterpret_cast<const char*>(buffer_.data()),
						buffer_.size() * sizeof(Pixel)
					);
				}
				else if (type == ModelType::Visual)
				{
					for (std::size_t j = 0; j < height_; j++)
					{
						for (std::size_t i = 0; i < width_; i++)
							file << at(i, j).v;
						file << '\n';
					}
				}
			}
		};
	}
	namespace style
	{
		struct Model
		{
			enum MaskMode
			{
				// Whether to apply individual masks

				ApplyFg = 1 << 0,
				ApplyBg = 1 << 1,

				// Whether to override or interpolate between existing colors

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
			public Element,
			public style::UAI<Matrix, style::ILayout, style::IModel>,
			public impl::UnitUpdateHelper<Matrix>
		{
		public:
			friend class UnitUpdateHelper;
			using data = style::UAI<Matrix, style::ILayout, style::IModel>;
		protected:
			virtual unit_meta_flag _unit_report_impl() const noexcept override
			{
				return UnitUpdateHelper::_report_units();
			}
			virtual char _index_impl() const noexcept override
			{
				return data::zindex;
			}

			virtual BoundingBox _dimensions_impl() const noexcept override
			{
				if (!data::model)
					return { 0, 0 };
				return {
					static_cast<int>(data::model->width()),
					static_cast<int>(data::model->height())
				};
			}
			virtual Position _position_impl() const noexcept override
			{
				return {
					_resolve_units(data::x),
					_resolve_units(data::y)
				};
			}

			virtual BoundingBox _reserve_buffer_impl() const noexcept override
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
			virtual PixelBuffer::View _fetch_pixel_buffer_impl() const noexcept override
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
			virtual bool _provides_buffer_impl() const noexcept override
			{
				return true;
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
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
		public:
			using Element::Element;
		};
	}
}

#endif // D2_MATRIX_HPP
