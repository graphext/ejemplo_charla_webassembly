#include "VectorString.h"
#include "LZipDecoder.h"

#ifdef __EMSCRIPTEN__
    #include "glue.cpp"
#else
#endif

#ifdef __EMSCRIPTEN__
int main(int argc, char** argv) {
	EM_ASM(
		window.dispatchEvent(new CustomEvent("WasmLoaded"))
	);
}
#else
int main(int argc, char** argv) {
	
}
#endif
