#include "FileHandler.hpp"

#include <fstream>

#include "Logger.hpp"

// Reads file data in the specified path
void mtd::FileHandler::readFile(const char* filePath, std::vector<char>& fileData)
{
	std::ifstream file{filePath, std::ios::binary};
	if(!file)
	{
		LOG_ERROR("File \"%s\" was not found.", filePath);
	}

	file.seekg(0, std::ios::end);
	fileData.resize(file.tellg());
	file.seekg(0, std::ios::beg);

	file.read(fileData.data(), fileData.size());
	file.close();
}
