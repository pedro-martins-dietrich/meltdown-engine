#pragma once

#include "../Material/Material.hpp"

namespace mtd::MaterialIO
{
	// Saves material data to a `.mtrl` file with the specified name, returning true if successful
	bool saveMaterial(std::string_view fileName, const Material& material);

	// Loads material data from a `.mtrl` file in the specified path, returning true if successful
	bool loadMaterial(std::string_view fileName, Material& material);
}
