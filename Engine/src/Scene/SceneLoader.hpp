#pragma once

#include <memory>

#include "InstanceManager.hpp"
#include "../AssetManager/TexturePool.hpp"
#include "../Vulkan/Descriptors/DescriptorManager.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"
#include "../Vulkan/Pipeline/PipelineBundles.hpp"

// Responsible to load Meltdown scenes from file
namespace mtd::SceneLoader
{
	// Loads all scene data from a Meltdown scene file
	void load
	(
		const Device& device,
		std::string_view fileName,
		std::vector<FramebufferInfo>& framebufferInfos,
		PipelineInfoBundle& pipelineInfos,
		std::vector<RenderPassInfo>& renderOrder,
		std::vector<std::unique_ptr<MeshManager>>& meshManagers,
		std::vector<MeshData>& meshes,
		ResourceManager& resourceManager,
		DescriptorManager& descriptorManager,
		InstanceManager& instanceManager,
		SceneResources& sceneResources,
		TexturePool& texturePool
	);
}
