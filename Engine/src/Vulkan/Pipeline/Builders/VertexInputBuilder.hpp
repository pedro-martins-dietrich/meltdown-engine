#pragma once

#include <vulkan/vulkan.hpp>

#include "../../../Utils/EngineEnums.hpp"

// Builder for the vertex input create info
namespace mtd::VertexInputBuilder
{
	// Configures a vertex input create info based on the pipeline type
	void setVertexInput
	(
		PipelineType type,
		vk::PipelineVertexInputStateCreateInfo& vertexInputInfo
	);
}
