#pragma once

#include <memory>

#include "../Vulkan/Mesh/MeshManager.hpp"

// Responsible to load Meltdown scenes from file
namespace mtd::SceneLoader
{
	// Loads the meshes from a Meltdown scene file
	void load
	(
		const Device& device,
		const char* fileName,
		std::vector<FramebufferInfo>& framebufferInfos,
		std::vector<PipelineInfo>& pipelineInfos,
		std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
		std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos,
		std::vector<RenderPassInfo>& renderOrder,
		std::vector<std::unique_ptr<MeshManager>>& meshManagers
	);
}
