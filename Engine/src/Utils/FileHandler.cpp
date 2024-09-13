#include <pch.hpp>
#include "FileHandler.hpp"

#include "Logger.hpp"

// Reads file data in the specified path
bool mtd::FileHandler::readFile(const char* filePath, std::vector<char>& fileData)
{
	std::ifstream file{filePath, std::ios::binary};
	if(!file)
	{
		LOG_ERROR("File \"%s\" was not found.", filePath);
		return false;
	}

	file.seekg(0, std::ios::end);
	fileData.resize(file.tellg());
	file.seekg(0, std::ios::beg);

	file.read(fileData.data(), fileData.size());
	file.close();

	return true;
}

// Reads a file and return its content as a JSON
bool mtd::FileHandler::readJSON(const char* filePath, nlohmann::json& json)
{
	std::vector<char> fileData;
	if(!readFile(filePath, fileData))
		return false;
	fileData.push_back('\0');

	try
	{
		json = nlohmann::json::parse(fileData.data());
	}
	catch(nlohmann::json::exception error)
	{
		LOG_ERROR
		(
			"Failed to read JSON file.\n\tError ID: %d\n\tDescription: \"%s\".\n",
			error.id,
			error.what()
		);
		return false;
	}

	return true;
}
