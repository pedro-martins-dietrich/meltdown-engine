#include <pch.hpp>
#include "ShaderLoader.hpp"

#include "../../../Utils/Logger.hpp"

// Shader modules loading for each pipeline type
static void loadDefaultShaders
(
	const vk::Device& device, std::vector<mtd::ShaderModule>& shaders
);
static void loadBillboardShaders
(
	const vk::Device& device, std::vector<mtd::ShaderModule>& shaders
);

// Loads the shader modules based on the pipeline type
void mtd::ShaderLoader::loadShaders
(
	PipelineType type, const vk::Device& device, std::vector<ShaderModule>& shaders
)
{
	switch(type)
	{
		case PipelineType::DEFAULT:
			loadDefaultShaders(device, shaders);
			break;
		case PipelineType::BILLBOARD:
			loadBillboardShaders(device, shaders);
			break;
		default:
			LOG_WARNING("No shaders have been specified for the pipeline type %d.", type);
			loadDefaultShaders(device, shaders);
	}
}

// Loads the shaders for the default pipeline
void loadDefaultShaders(const vk::Device& device, std::vector<mtd::ShaderModule>& shaders)
{
	shaders.reserve(2);
	shaders.emplace_back("default.vert.spv", device);
	shaders.emplace_back("default.frag.spv", device);
}

// Loads the shaders for the billboard pipeline
void loadBillboardShaders(const vk::Device& device, std::vector<mtd::ShaderModule>& shaders)
{
	shaders.reserve(2);
	shaders.emplace_back("billboard.vert.spv", device);
	shaders.emplace_back("billboard.frag.spv", device);
}
