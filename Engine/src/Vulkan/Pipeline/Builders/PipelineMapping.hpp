#pragma once

#include <vulkan/vulkan.hpp>

#include <meltdown/enums.hpp>

// Maps the engine's pipeline info enums to the Vulkan enums
namespace mtd::PipelineMapping
{
	// Gets the Vulkan primitive topology enum
	vk::PrimitiveTopology mapPrimitiveTopology(ShaderPrimitiveTopology primitiveTopology);
	// Gets the Vulkan polygon mode enum
	vk::PolygonMode mapPolygonMode(ShaderPrimitiveTopology primitiveTopology);
	// Gets the Vulkan cull mode flags
	vk::CullModeFlags mapCullModeFlags(ShaderFaceCulling faceCulling);
	// Gets the Vulkan front face enum
	vk::FrontFace mapFrontFace(ShaderFaceCulling faceCulling);
	// Gets the Vulkan buffer usage from the descriptor type
	vk::BufferUsageFlags mapBufferUsageFlags(DescriptorType descriptorType);
}
