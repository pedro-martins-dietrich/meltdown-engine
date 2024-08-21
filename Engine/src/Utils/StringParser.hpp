#pragma once

#include <string>
#include <vector>

// Common string manipulation functions
namespace mtd::StringParser
{
	// Separates a line into substrings, using the specified delimiter
	void split(std::string line, std::string delimiter, std::vector<std::string>& output);
}
