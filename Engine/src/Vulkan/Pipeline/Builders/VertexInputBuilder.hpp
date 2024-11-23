#pragma once

#include <vulkan/vulkan.hpp>

#include <meltdown/enums.hpp>

// Builder for the vertex input create info
namespace mtd::VertexInputBuilder
{
	// Configures a vertex input create info based on the mesh type
	void setVertexInput(MeshType type, vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
}
