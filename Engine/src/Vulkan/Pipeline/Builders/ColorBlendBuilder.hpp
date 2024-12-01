#pragma once

#include <vulkan/vulkan.hpp>

// Builder for the color blend create info
namespace mtd::ColorBlendBuilder
{
	// Configures the color blending create info based on the pipeline type
	void setColorBlending
	(
		bool useTransparency,
		vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
		vk::PipelineColorBlendAttachmentState& colorBlendAttachment
	);
}
