#ifndef D2_MODEL_HPP
#define D2_MODEL_HPP

#include "d2_pixel.hpp"
#include "d2_exceptions.hpp"
#include <memory>

namespace d2
{
	class MatrixModel : public PixelBuffer
	{
        D2_TAG_MODULE(matrix)
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
		static MatrixModel::ptr make(int width = 0, int height = 0);

		void load_model(const std::string& path, ModelType type);
		void save_model(const std::string& path, ModelType type);
	};

}

#endif // D2_MODEL_HPP
