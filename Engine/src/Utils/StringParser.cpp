#include <pch.hpp>
#include "StringParser.hpp"

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

std::string mtd::StringParser::getFileStem(std::string_view filePath, bool keepSuffix)
{
	if(filePath.empty()) return {};

	size_t lastSeparator = filePath.find_last_of("/\\");
	std::string_view fileName =
		(lastSeparator == std::string_view::npos) ? filePath : filePath.substr(lastSeparator + 1);
	if(fileName.empty()) return {};

	if(keepSuffix) return std::string{fileName};

	size_t lastDot = fileName.find_last_of('.');
	if(lastDot == std::string_view::npos || lastDot == 0)
		return std::string{fileName};

	return std::string{fileName.substr(0, lastDot)};
}
