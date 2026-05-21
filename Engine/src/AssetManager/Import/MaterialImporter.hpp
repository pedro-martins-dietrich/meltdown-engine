#pragma once

#include <vector>

// Handles logic for loading a material from a .mtrl file
namespace mtd::MaterialImporter
{
    // Loads a .mtrl file and appends it to the material data
    bool loadMaterial(std::string_view filePath, std::vector<std::byte>& materialData);
}
