#include "VertexInputBuilder.hpp"

#include "../../../Utils/EngineStructs.hpp"
#include "../../../Utils/Logger.hpp"

// Vertex input builders for each pipeline type
static void defaultVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
static void noVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);

// Creates a vertex input create info based on the pipeline type
void mtd::VertexInputBuilder::setVertexInput
(
	PipelineType type,
	vk::PipelineVertexInputStateCreateInfo& vertexInputInfo
)
{
	switch(type)
	{
		case PipelineType::DEFAULT:
			defaultVertexInput(vertexInputInfo);
			break;

		case PipelineType::BILLBOARD:
			noVertexInput(vertexInputInfo);
			break;

		default:
			LOG_WARNING("No vertex input builder set for pipeline type %d.", type);
			noVertexInput(vertexInputInfo);
	}
}

// Uses mesh data for vertex input
void defaultVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	static vk::VertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = static_cast<uint32_t>(sizeof(mtd::Vertex));
	bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	static std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions;
	// Position
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = 0;
	// Texture coordinate
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[1].offset = 3 * sizeof(float);
	// Normal vector
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = 5 * sizeof(float);
	// Color
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = 8 * sizeof(float);

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

// Sets no vertex input for pipeline
void noVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
}
