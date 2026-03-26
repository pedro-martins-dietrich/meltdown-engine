#pragma once

#include <fstream>
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
	bool readFile(const char* filePath, std::vector<char>& fileData);

	// Reads a file and return its content as a JSON
	bool readJSON(const char* filePath, nlohmann::json& json);

	// Reads a string at the current cursor location from an open file
	bool readString(std::ifstream& openFile, std::string& str);

	// Writes a string to an open file
	bool writeString(std::ofstream& openFile, std::string_view str);
}
