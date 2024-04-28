#include "StringParser.hpp"

// Separates a line into substrings, using the specified delimiter
void mtd::StringParser::split
(
	std::string line, std::string delimiter, std::vector<std::string>& output
)
{
	output.clear();

	size_t pos = line.find(delimiter);
	while(pos != std::string::npos)
	{
		output.push_back(line.substr(0, pos));
		line.erase(0, pos + delimiter.length());

		pos = line.find(delimiter);
	}

	output.push_back(line);
}
