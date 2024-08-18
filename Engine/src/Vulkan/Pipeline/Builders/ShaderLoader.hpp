#pragma once

#include "../../../Utils/EngineEnums.hpp"
#include "../ShaderModule.hpp"

// Handles shader modules loading
namespace mtd::ShaderLoader
{
	// Loads the shader modules based on the pipeline type
	void loadShaders
	(
		PipelineType type, const vk::Device& device, std::vector<ShaderModule>& shaders
	);
}
