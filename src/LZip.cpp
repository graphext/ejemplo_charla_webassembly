#include "LZip.h"

#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>
#include <chrono>

LZipDecoder::LZipDecoder(const char* dataIn, const uint32_t sizeIn)
	: compressedSize(sizeIn)
{
	const auto start = std::chrono::steady_clock::now();

	uncompressedSize = *reinterpret_cast<const uint32_t*>(dataIn);
	dataIn += sizeof(uint32_t);
	uint8_t* buffer = new uint8_t[uncompressedSize];

	auto decoder = LZ_decompress_open();
	bool error = false, finishing = false;
	uint32_t written = 0, read = 0;

	while(!error && read < uncompressedSize) {
		const auto availableWrite = LZ_decompress_write_size(decoder);

		if(!finishing && availableWrite > 0) {
			if(written < sizeIn - sizeof(uint32_t)) {
				int32_t toBeWritten = sizeIn - sizeof(uint32_t) - written;

				const int32_t wr = LZ_decompress_write(decoder, reinterpret_cast<const uint8_t*>(dataIn), toBeWritten);
				if(wr < 0) {
					error = true;
					std::cerr << "Error on LZ_decompress_write.\n";
					break;
				}

				written += wr;
				dataIn += wr;
			}
			else {
				LZ_decompress_finish(decoder);
				finishing = true;
			}
		}
		else {
			for(;;) {
				int32_t rd = LZ_decompress_read(decoder, buffer + read, uncompressedSize - read);
				if(rd < 0) {
					error = true;
					std::cerr << "Error on LZ_decompress_read.\n";
					break;
				}
				else if(rd == 0) {
					break;
				}

				read += rd;
			}
		}
	}

	if(LZ_decompress_close(decoder) < 0) {
		error = true;
	}

	if(error) {
		std::cerr << "Error decompressing\n";
		if(buffer != nullptr) {
			delete[] buffer;
		}
		return;
	}
	else {
		uncompressedData.reset(buffer, std::default_delete<uint8_t[]>());
	}

	const auto end = std::chrono::steady_clock::now();
	const auto millis = uint32_t(std::chrono::duration<double, std::milli>(end - start).count());

	std::cout <<
		"Decompression: " << uint32_t(std::round(compressedSize/1024.0/1024.0))
		<< "MB -> " << uint32_t(std::round(uncompressedSize/1024.0/1024.0))
		<< "MB (ratio: " << 1.0*compressedSize/uncompressedSize << ", time: " << millis << "ms)\n";
}

LZipEncoder::LZipEncoder(const char* dataIn, const uint32_t sizeIn, uint32_t matchLenLimit)
	: uncompressedSize(sizeIn)
{
	const auto start = std::chrono::steady_clock::now();

	matchLenLimit = std::max(matchLenLimit, 5U);
	bool error = false;
	uint32_t written = 0;

	compressedData.resize(4);
	*reinterpret_cast<uint32_t*>(&compressedData.front()) = uncompressedSize;

	LZ_Encoder* encoder = LZ_compress_open(1 << 20, matchLenLimit, std::numeric_limits<uint64_t>::max());

	if(!encoder || LZ_compress_errno(encoder) != LZ_ok) {
		LZ_compress_close(encoder);
		std::cerr << "Error while initializing compression.\n";
		return;
	}

	while(!error && written < uncompressedSize) {
		if(LZ_compress_write_size(encoder) > 0) {
			const int32_t wr = LZ_compress_write(encoder, reinterpret_cast<const uint8_t*>(dataIn) + written, uncompressedSize - written);
			if(wr < 0) {
				error = true;
				std::cerr << "Error on LZ_compress_write.\n";
				break;
			}
			written += wr;
		}
		else {
			error = !readCompressed(encoder);
		}
	}

	if(error) {
		std::cerr << "Error while compressing.\n";
		return;
	}

	error = !readCompressed(encoder) || LZ_compress_finish(encoder) < 0 || !readCompressed(encoder);
	error = LZ_compress_close(encoder) < 0 || error;

	if(error) {
		std::cerr << "Error while finalizing compression.\n";
	}

	const auto end = std::chrono::steady_clock::now();
	const auto millis = uint32_t(std::chrono::duration<double, std::milli>(end - start).count());

	std::cout <<
		"Compression: " << uint32_t(std::round(getUncompressedSize()/1024.0/1024.0))
		<< "MB -> " << uint32_t(std::round(getCompressedSize()/1024.0/1024.0))
		<< "MB (ratio: " << 1.0*getCompressedSize()/getUncompressedSize() << ", time: " << millis << "ms)\n";
}

bool LZipEncoder::readCompressed(LZ_Encoder* encoder) {
	std::array<uint8_t, 70 * 1024> outTmp;
	bool error = false;

	for(;;) {
		int32_t rd = LZ_compress_read(encoder, outTmp.data(), outTmp.size());
		if(rd < 0) {
			error = true;
			std::cerr << "Error on LZ_compress_read.\n";
			break;
		}
		else if(rd == 0) {
			break;
		}
		compressedData.insert(compressedData.end(), outTmp.data(), outTmp.data() + rd);
	}

	return !error;
}
