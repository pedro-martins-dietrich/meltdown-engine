#include <pch.hpp>
#include "FileHandler.hpp"

#include "Logger.hpp"

bool mtd::FileHandler::readFile(const char* filePath, std::vector<char>& fileData)
{
	std::ifstream file{filePath, std::ios::binary | std::ios::ate};
	if(!file)
	{
		LOG_ERROR("File \"%s\" was not found.", filePath);
		return false;
	}

	fileData.resize(file.tellg());
	file.seekg(0, std::ios::beg);

	file.read(fileData.data(), fileData.size());
	file.close();

	return true;
}

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

bool mtd::FileHandler::readString(std::ifstream& openFile, std::string& str)
{
	uint32_t strSize = 0U;

	openFile.read(reinterpret_cast<char*>(&strSize), sizeof(uint32_t));
	if(!openFile) return false;

	str.resize(strSize);
	openFile.read(str.data(), static_cast<std::streamsize>(strSize));

	return openFile.good();
}

bool mtd::FileHandler::writeString(std::ofstream& openFile, std::string_view str)
{
	uint32_t strSize = static_cast<uint32_t>(str.size());
	openFile.write(reinterpret_cast<char*>(&strSize), sizeof(uint32_t));
	openFile.write(str.data(), static_cast<std::streamsize>(strSize));

	return openFile.good();
}
