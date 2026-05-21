#pragma once

#include <memory>

#include "../AssetManager/TexturePool.hpp"
#include "../AssetManager/MaterialManager.hpp"
#include "../AssetManager/MeshPool.hpp"
#include "../Vulkan/Mesh/MeshManager.hpp"
#include "../Vulkan/Pipeline/PipelineBundles.hpp"

// Responsible to load Meltdown scenes from file
namespace mtd::SceneLoader
{
	// Loads the meshes from a Meltdown scene file
	void load
	(
		const Device& device,
		std::string_view fileName,
		std::vector<FramebufferInfo>& framebufferInfos,
		PipelineInfoBundle& pipelineInfos,
		std::vector<RenderPassInfo>& renderOrder,
		std::vector<std::unique_ptr<MeshManager>>& meshManagers,
		TexturePool& texturePool,
		MaterialManager& materialManager,
		MeshPool& meshPool
	);
}
