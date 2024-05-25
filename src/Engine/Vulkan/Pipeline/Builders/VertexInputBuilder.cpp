#include "VertexInputBuilder.hpp"

#include "../../../Utils/Logger.hpp"
#include "../../Mesh/Mesh.hpp"

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
	const vk::VertexInputBindingDescription& bindingDescription =
		mtd::Mesh::getInputBindingDescription();
	const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions =
		mtd::Mesh::getInputAttributeDescriptions();

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
