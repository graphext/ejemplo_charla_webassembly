#pragma once

#include <vector>
#include <string>

class VectorString : public std::vector<std::string> {
	using stdVector = std::vector<std::string>;
	using stdVector::stdVector;

	public:
		const char* at(uint32_t i) const {
			return stdVector::at(i).c_str();
		}
		void set(uint32_t i, const char* str) {
			(*this)[i] = str;
		}
};
