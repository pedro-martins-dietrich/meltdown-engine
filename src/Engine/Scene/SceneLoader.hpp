#pragma once

#include "../Vulkan/Mesh/Mesh.hpp"

// Responsible to load Meltdown scenes from file
namespace mtd::SceneLoader
{
	// Loads the meshes from a Meltdown scene file
	void load(const char* fileName, std::vector<Mesh>& meshes);
}
