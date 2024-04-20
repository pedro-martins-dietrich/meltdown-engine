#pragma once

#include <string>
#include <vector>

// Handles file manipulation
namespace mtd::FileHandler
{
	// Reads file data in the specified path
	void readFile(const char* filePath, std::vector<char>& fileData);
}
