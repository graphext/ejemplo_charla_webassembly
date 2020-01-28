#pragma once

#include "lzip/lzlib.h"

#include <memory>

class LZipDecoder {
	public:
		LZipDecoder(const uint8_t* data, const uint32_t sizeIn);
		uint32_t getCompressedSize() const { return compressedSize; }
		uint32_t getUncompressedSize() const { return uncompressedSize; }

	private:
		uint32_t compressedSize = 0;
		uint32_t uncompressedSize = 0;
		std::shared_ptr<uint8_t> uncompressedData = nullptr;
};
