#pragma once

#include "InstanceManager.hpp"
#include "../Vulkan/AccelerationStructure/RayTracingBVH.hpp"
#include "../Vulkan/Descriptors/DescriptorManager.hpp"
#include "../Vulkan/Pipeline/PipelineBundles.hpp"

// Responsible to load Meltdown scenes from file
namespace mtd::SceneLoader
{
	// Loads all scene data from a Meltdown scene file
	void load
	(
		const Device& mtdDevice,
		std::string_view fileName,
		std::vector<FramebufferInfo>& framebufferInfos,
		PipelineInfoBundle& pipelineInfos,
		std::vector<RenderPassInfo>& renderOrder,
		std::vector<MeshData>& meshes,
		ResourceManager& resourceManager,
		DescriptorManager& descriptorManager,
		InstanceManager& instanceManager,
		std::optional<RayTracingBVH>& rayTracingBVH,
		SceneResources& sceneResources
	);
}
