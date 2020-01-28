#include "LZipDecoder.h"

#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>
#include <chrono>

LZipDecoder::LZipDecoder(const uint8_t* dataIn, const uint32_t sizeIn)
	: compressedSize(sizeIn)
{
	std::array<uint8_t, 70 * 1024> outTmp;
	uint8_t* buffer = outTmp.data();

	const auto start = std::chrono::steady_clock::now();

	auto decoder = LZ_decompress_open();
	bool error = false, finishing = false;
	uint32_t written = 0, read = 0;

	while(!error && (read < uncompressedSize || read == 0)) {
		const auto availableWrite = LZ_decompress_write_size(decoder);

		if(!finishing && availableWrite > 0) {
			if(written < compressedSize) {
				int32_t toBeWritten = compressedSize - written;

				const int32_t wr = LZ_decompress_write(decoder, dataIn, toBeWritten);
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
				else if(read == 0) {
					uncompressedSize = LZ_decompress_total_out_size(decoder);
					buffer = new uint8_t[uncompressedSize];
					std::copy(outTmp.data(), outTmp.data() + rd, buffer);
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
		if(buffer != outTmp.data()) {
			delete[] buffer;
		}
		return;
	}
	else {
		uncompressedData.reset(buffer);
	}

	const auto end = std::chrono::steady_clock::now();
	const auto millis = uint32_t(std::chrono::duration<double, std::milli>(end - start).count());

	std::cout << 
		"Decompression: " << uint32_t(std::round(compressedSize/1024.0/1024.0))
		<< "MB -> " << uint32_t(std::round(uncompressedSize/1024.0/1024.0))
		<< "MB (ratio: " << 1.0*compressedSize/uncompressedSize << ", time: " << millis << "ms)\n";
}
