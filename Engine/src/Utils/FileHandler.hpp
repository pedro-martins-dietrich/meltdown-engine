#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#ifndef MTD_RESOURCES_PATH
	#define MTD_RESOURCES_PATH "./resources/"
#endif

// Handles file manipulation
namespace mtd::FileHandler
{
	// Reads file data in the specified path
	void readFile(const char* filePath, std::vector<char>& fileData);

	// Reads a file and return its content as a JSON
	bool readJSON(const char* filePath, nlohmann::json& json);
}
