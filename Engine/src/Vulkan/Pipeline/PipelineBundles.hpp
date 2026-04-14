#pragma once

#include "GraphicsPipeline.hpp"
#include "FramebufferPipeline.hpp"

namespace mtd
{
	// Bundle with all pipelines
	struct PipelineBundle
	{
		std::vector<GraphicsPipeline> graphicsPipelines;
		std::vector<ComputePipeline> computePipelines;
		std::vector<RayTracingPipeline> rayTracingPipelines;
		std::vector<FramebufferPipeline> framebufferPipelines;
	};

	// Bundle with all pipeline infos
	struct PipelineInfoBundle
	{
		std::vector<GraphicsPipelineInfo> graphicsInfos;
		std::vector<ComputePipelineInfo> computeInfos;
		std::vector<RayTracingPipelineInfo> rayTracingInfos;
		std::vector<FramebufferPipelineInfo> framebufferInfos;
	};
}
