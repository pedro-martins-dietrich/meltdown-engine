#include <pch.hpp>
#include "VertexInputBuilder.hpp"

#include "../../../Utils/EngineStructs.hpp"
#include "../../../Utils/Logger.hpp"

// Vertex input builders for each pipeline type
static void defaultVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);
static void noVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo);

// Configures a vertex input create info based on the mesh type
void mtd::VertexInputBuilder::setVertexInput(MeshType type, vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	switch(type)
	{
		case MeshType::Default3D:
		case MeshType::MultiMaterial3D:
			defaultVertexInput(vertexInputInfo);
			break;

		case MeshType::Billboard:
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
	static std::array<vk::VertexInputBindingDescription, 2> bindingDescriptions;
	// Per vertex binding
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = static_cast<uint32_t>(sizeof(mtd::Vertex));
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
	// Per instance binding
	bindingDescriptions[1].binding = 1;
	bindingDescriptions[1].stride = static_cast<uint32_t>(sizeof(mtd::Mat4x4));
	bindingDescriptions[1].inputRate = vk::VertexInputRate::eInstance;

	static std::array<vk::VertexInputAttributeDescription, 7> attributeDescriptions;
	// Vertex position
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = 0;
	// Vertex texture coordinate
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[1].offset = 3 * sizeof(float);
	// Vertex normal vector
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = 5 * sizeof(float);
	// Instance transformation matrix
	for(size_t i = 0; i < 4; i++)
	{
		attributeDescriptions[3 + i].location = 4 + i;
		attributeDescriptions[3 + i].binding = 1;
		attributeDescriptions[3 + i].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[3 + i].offset = 4 * i * sizeof(float);
	}

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

// Sets no vertex input for pipeline
void noVertexInput(vk::PipelineVertexInputStateCreateInfo& vertexInputInfo)
{
	static vk::VertexInputBindingDescription bindingDescription;
	// Per instance binding
	bindingDescription.binding = 0;
	bindingDescription.stride = static_cast<uint32_t>(sizeof(mtd::Mat4x4));
	bindingDescription.inputRate = vk::VertexInputRate::eInstance;

	static std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions;
	// Instance transformation matrix
	for(size_t i = 0; i < 4; i++)
	{
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[i].offset = 4 * i * sizeof(float);
	}

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}
