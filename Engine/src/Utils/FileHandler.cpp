#include <pch.hpp>
#include "FileHandler.hpp"

#include <stb_image.h>

#include "Logger.hpp"

bool mtd::FileHandler::readFile(std::string_view filePath, std::vector<char>& fileData)
{
	std::ifstream file{filePath.data(), std::ios::binary | std::ios::ate};
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

bool mtd::FileHandler::readJSON(std::string_view filePath, nlohmann::json& json)
{
	std::vector<char> fileData;
	if(!readFile(filePath, fileData))
	{
		LOG_ERROR("Failed to read JSON file: \"%s\".", filePath.data());
		return false;
	}
	fileData.push_back('\0');

	json = nlohmann::json::parse(fileData.data(), nullptr, false);
	if(json.is_discarded() || !json.is_object())
	{
		LOG_ERROR("Failed to parse JSON file: \"%s\".", filePath.data());
		return false;
	}

	return true;
}

void* mtd::FileHandler::readImage(std::string_view path, UIntVec2& dimensions, uint32_t& channels)
{
	int w, h, c;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load(path.data(), &w, &h, &c, STBI_default);
	if(!pixels)
	{
		LOG_ERROR("Failed to read image file: \"%s\".", path.data());
		return nullptr;
	}

	dimensions = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
	channels = static_cast<uint32_t>(c);

	return pixels;
}
