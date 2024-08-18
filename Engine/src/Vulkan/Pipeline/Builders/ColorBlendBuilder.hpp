#pragma once

#include <vulkan/vulkan.hpp>

#include "../../../Utils/EngineEnums.hpp"

// Builder for the color blend create info
namespace mtd::ColorBlendBuilder
{
	// Configures the color blending create info based on the pipeline type
	void setColorBlending
	(
		PipelineType type,
		vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
		vk::PipelineColorBlendAttachmentState& colorBlendAttachment
	);
}
