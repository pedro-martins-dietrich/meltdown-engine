#include <pch.hpp>
#include "VertexInputBuilder.hpp"

#include "../../../Utils/EngineStructs.hpp"
#include "../../../Utils/Logger.hpp"

namespace mtd::VertexInputBuilder
{
	// Vertex input builders for each pipeline type
	static void defaultVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
	static void billboardVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
}

void mtd::VertexInputBuilder::setVertexInput(MeshType type, vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	switch(type)
	{
		case MeshType::Default3D:
		case MeshType::MultiMaterial3D:
			defaultVertexInput(vertexInputInfo);
			break;

		case MeshType::Billboard:
			billboardVertexInput(vertexInputInfo);
			break;

		default:
			LOG_WARNING("No vertex input builder set for pipeline type %d.", type);
			defaultVertexInput(vertexInputInfo);
	}
}

void mtd::VertexInputBuilder::defaultVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	static std::array<vk::VertexInputBindingDescription, 2> bindingDescriptions;
	// Per vertex binding
	bindingDescriptions[0].binding = 0U;
	bindingDescriptions[0].stride = static_cast<uint32_t>(sizeof(Vertex));
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
	// Per instance binding
	bindingDescriptions[1].binding = 1U;
	bindingDescriptions[1].stride = static_cast<uint32_t>(sizeof(RenderObject));
	bindingDescriptions[1].inputRate = vk::VertexInputRate::eInstance;

	static std::array<vk::VertexInputAttributeDescription, 8> attributeDescriptions;
	// Vertex position
	attributeDescriptions[0].location = 0U;
	attributeDescriptions[0].binding = 0U;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = 0U;
	// Vertex texture coordinate
	attributeDescriptions[1].location = 1U;
	attributeDescriptions[1].binding = 0U;
	attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[1].offset = 3U * sizeof(float);
	// Vertex normal vector
	attributeDescriptions[2].location = 2U;
	attributeDescriptions[2].binding = 0U;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = 5U * sizeof(float);
	// Instance transformation matrix
	for(uint32_t i = 0U; i < 4U; i++)
	{
		attributeDescriptions[3 + i].location = 3U + i;
		attributeDescriptions[3 + i].binding = 1U;
		attributeDescriptions[3 + i].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[3 + i].offset = 4U * i * sizeof(float);
	}
	// Instance material set
	attributeDescriptions[7].location = 7U;
	attributeDescriptions[7].binding = 1U;
	attributeDescriptions[7].format = vk::Format::eR8G8B8A8Uint;
	attributeDescriptions[7].offset = 16U * sizeof(float);

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

void mtd::VertexInputBuilder::billboardVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	static std::array<vk::VertexInputBindingDescription, 2> bindingDescriptions;
	// Per vertex binding
	bindingDescriptions[0].binding = 0U;
	bindingDescriptions[0].stride = static_cast<uint32_t>(sizeof(Vec2));
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
	// Per instance binding
	bindingDescriptions[1].binding = 1U;
	bindingDescriptions[1].stride = static_cast<uint32_t>(sizeof(RenderObject));
	bindingDescriptions[1].inputRate = vk::VertexInputRate::eInstance;

	static std::array<vk::VertexInputAttributeDescription, 6> attributeDescriptions;
	// Vertex position
	attributeDescriptions[0].location = 0U;
	attributeDescriptions[0].binding = 0U;
	attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[0].offset = 0U;
	// Instance transformation matrix
	for(uint32_t i = 0U; i < 4U; i++)
	{
		attributeDescriptions[1 + i].location = 1U + i;
		attributeDescriptions[1 + i].binding = 1U;
		attributeDescriptions[1 + i].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[1 + i].offset = 4U * i * sizeof(float);
	}
	// Instance material set
	attributeDescriptions[5].location = 5U;
	attributeDescriptions[5].binding = 1U;
	attributeDescriptions[5].format = vk::Format::eR8G8B8A8Uint;
	attributeDescriptions[5].offset = 16U * sizeof(float);

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}
