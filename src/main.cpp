#include "VectorString.h"
#include "LZipDecoder.h"

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
	std::ifstream file("prueba.lz", std::ios::binary);

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	auto data = new char[size];
	file.read(data, size);

	LZipDecoder decoder(data, size);
	const auto uncompressedSize = decoder.getUncompressedSize();
	std::cout << decoder.getCompressedSize() << std::endl;
	std::cout << uncompressedSize << std::endl;
	//std::cout << decoder.getUncompressedAsCString() << std::endl;
}
#endif
