#include "VectorString.h"
#include "LZip.h"

#ifdef __EMSCRIPTEN__
	#include "glue.cpp"
#else
	#include <iostream>
	#include <fstream>
#endif

#ifdef __EMSCRIPTEN__
int main(int argc, char** argv) {
	EM_ASM(
		window.dispatchEvent(new CustomEvent("WasmLoaded"))
	);
}
#else
int main(int argc, char** argv) {
	std::ifstream file(argv[1], std::ios::binary);

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	auto data = new char[size];
	file.read(data, size);

	LZipEncoder encoder(data, size);
	delete[] data;

	std::cout << encoder.getCompressedSize() << std::endl;
	std::cout << encoder.getUncompressedSize() << std::endl;

	LZipDecoder decoder(encoder.getCompressedData(), encoder.getCompressedSize());
	std::cout << decoder.getCompressedSize() << std::endl;
	std::cout << decoder.getUncompressedSize() << std::endl;
	//std::cout << decoder.getUncompressedAsCString() << std::endl;
}
#endif
