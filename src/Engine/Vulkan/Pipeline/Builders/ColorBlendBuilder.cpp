#include "ColorBlendBuilder.hpp"

#include "../../../Utils/Logger.hpp"

// Color blending builders for each pipeline type
static void opaqueColorBlending
(
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
);
static void transparentColorBlending
(
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
);

// Configures the color blending create info based on the pipeline type
void mtd::ColorBlendBuilder::setColorBlending
(
	mtd::PipelineType type,
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
)
{
	switch(type)
	{
		case PipelineType::DEFAULT:
			opaqueColorBlending(colorBlendInfo, colorBlendAttachment);
			break;

		case PipelineType::BILLBOARD:
			transparentColorBlending(colorBlendInfo, colorBlendAttachment);
			break;

		default:
			LOG_WARNING("No color blending builder set for pipeline type %d.", type);
			opaqueColorBlending(colorBlendInfo, colorBlendAttachment);
	}
}

// Does not use color blending (opaque objects)
void opaqueColorBlending
(
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
)
{
	colorBlendAttachment.blendEnable = vk::False;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	colorBlendInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlendInfo.logicOpEnable = vk::False;
	colorBlendInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
}

// Enables transparent objects
void transparentColorBlending
(
	vk::PipelineColorBlendStateCreateInfo& colorBlendInfo,
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment
)
{
	colorBlendAttachment.blendEnable = vk::True;
	colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	colorBlendInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlendInfo.logicOpEnable = vk::False;
	colorBlendInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
}
