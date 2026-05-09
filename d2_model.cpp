#include "d2_model.hpp"
#include <filesystem>
#include <fstream>

namespace d2
{
	MatrixModel::ptr MatrixModel::make(int width, int height)
	{
		auto ptr = std::make_shared<MatrixModel>();
		ptr->set_size(width, height);
		return ptr;
	}

	void MatrixModel::load_model(const std::string& path, ModelType type)
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
                D2_THRW("Attempt to load invalid model");

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
                D2_THRW("Attempt to load invalid model; size mismatch");

			if (header & Compressed)
			{
                D2_THRW("Compression is not implemented yet");
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

	void MatrixModel::save_model(const std::string& path, ModelType type)
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
}
