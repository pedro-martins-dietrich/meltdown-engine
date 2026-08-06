#pragma once

#include <nlohmann/json.hpp>

#include <meltdown/math.hpp>

#ifndef MTD_RESOURCES_PATH
	#define MTD_RESOURCES_PATH "./resources/"
#endif

// Handles file manipulation
namespace mtd::FileHandler
{
	// Reads file data in the specified path
	bool readFile(std::string_view filePath, std::vector<char>& fileData);

	// Reads a file and return its content as a JSON
	bool readJSON(std::string_view filePath, nlohmann::json& json);

	// Reads an image file and returns the pointer data or a `nullptr` if it fails
	void* readImage(std::string_view path, UIntVec2& dimensions, uint32_t& channels);
}
