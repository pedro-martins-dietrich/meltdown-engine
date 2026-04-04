#pragma once

#include "GraphicsPipeline.hpp"
#include "FramebufferPipeline.hpp"

namespace mtd
{
	// Bundle with all pipelines
	struct PipelineBundle
	{
		std::vector<GraphicsPipeline> graphicsPipelines;
		std::vector<FramebufferPipeline> framebufferPipelines;
		std::vector<RayTracingPipeline> rayTracingPipelines;
	};

	// Bundle with all pipeline infos
	struct PipelineInfoBundle
	{
		std::vector<GraphicsPipelineInfo> graphicsInfos;
		std::vector<FramebufferPipelineInfo> framebufferInfos;
		std::vector<RayTracingPipelineInfo> rayTracingInfos;
	};
}
