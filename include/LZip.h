#pragma once

#include "lzip/lzlib.h"

#include <memory>
#include <vector>

class LZipDecoder {
	public:
		LZipDecoder(const char* data, const uint32_t sizeIn);
		uint32_t getCompressedSize() const { return compressedSize; }
		uint32_t getUncompressedSize() const { return uncompressedSize; }
		const char* getUncompressedAsCString() const { return reinterpret_cast<const char*>(uncompressedData.get()); }

	private:
		uint32_t compressedSize = 0;
		uint32_t uncompressedSize = 0;
		std::shared_ptr<uint8_t> uncompressedData = nullptr;
};

class LZipEncoder {
	public:
		LZipEncoder(const char* data, const uint32_t sizeIn, uint32_t matchLenLimit = 13);
		uint32_t getCompressedSize() const { return compressedData.size(); }
		uint32_t getUncompressedSize() const { return uncompressedSize; }
		const char* getCompressedData() const { return reinterpret_cast<const char*>(compressedData.data()); }
		uint32_t getJsPtr() const { return reinterpret_cast<uint32_t>(compressedData.data()); };

	private:
		uint32_t uncompressedSize = 0;
		std::vector<uint8_t> compressedData;
		bool readCompressed(LZ_Encoder* encoder);
};
