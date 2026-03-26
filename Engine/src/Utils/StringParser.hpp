#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace mtd
{
	// Common string manipulation functions
	namespace StringParser
	{
		// Separates a line into substrings, using the specified delimiter
		void split(std::string line, std::string delimiter, std::vector<std::string>& output);

		// Extracts the file stem from a file path
		std::string getFileStem(std::string_view filePath, bool keepSuffix);
	}

	// Converts an 4-character string literal to an uint32_t
	constexpr uint32_t operator""_u32(const char* str, size_t size)
	{
		if(size != sizeof(uint32_t))
			throw "Literal must contain exactly 4 characters.";

		uint32_t value = 0x0;
		for(size_t i = 0; i < size; i++)
			value |= static_cast<uint32_t>(str[i]) << (8 * i);
		return value;
	}

	// Converts an 8-character string literal to an uint64_t
	constexpr uint64_t operator""_u64(const char* str, size_t size)
	{
		if(size != sizeof(uint64_t))
			throw "Literal must contain exactly 8 characters.";

		uint64_t value = 0x0;
		for(size_t i = 0; i < size; i++)
			value |= static_cast<uint64_t>(str[i]) << (8 * i);
		return value;
	}
}
