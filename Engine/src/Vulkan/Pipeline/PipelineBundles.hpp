#pragma once

#include "RasterizationPipeline.hpp"
#include "FramebufferPipeline.hpp"

namespace mtd
{
	// Bundle with all pipelines currently in use
	struct PipelineBundle
	{
		std::vector<RasterizationPipeline> rasterizationPipelines;
		std::vector<ComputePipeline> computePipelines;
		std::vector<RayTracingPipeline> rayTracingPipelines;
		std::vector<FramebufferPipeline> framebufferPipelines;
	};

	// Bundle with all pipeline infos
	struct PipelineInfoBundle
	{
		std::vector<RasterizationPipelineInfo> rasterizerInfos;
		std::vector<ComputePipelineInfo> computeInfos;
		std::vector<RayTracingPipelineInfo> rayTracingInfos;
		std::vector<FramebufferPipelineInfo> framebufferInfos;
	};
}
