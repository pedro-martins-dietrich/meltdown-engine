#include <pch.hpp>
#include "PipelineMapping.hpp"

#include "../../../Utils/Logger.hpp"

// Gets the Vulkan primitive topology enum.
vk::PrimitiveTopology mtd::PipelineMapping::mapPrimitiveTopology(ShaderPrimitiveTopology primitiveTopology)
{
	switch(primitiveTopology)
	{
		case ShaderPrimitiveTopology::PointList:
			return vk::PrimitiveTopology::ePointList;
		case ShaderPrimitiveTopology::LineList:
			return vk::PrimitiveTopology::eLineList;
		case ShaderPrimitiveTopology::LineStrip:
			return vk::PrimitiveTopology::eLineStrip;
		case ShaderPrimitiveTopology::LineListAdjacent:
			return vk::PrimitiveTopology::eLineListWithAdjacency;
		case ShaderPrimitiveTopology::LineStripAdjacent:
			return vk::PrimitiveTopology::eLineStripWithAdjacency;
		case ShaderPrimitiveTopology::TriangleList:
			return vk::PrimitiveTopology::eTriangleList;
		case ShaderPrimitiveTopology::TriangleStrip:
			return vk::PrimitiveTopology::eTriangleStrip;
		case ShaderPrimitiveTopology::TriangleFan:
			return vk::PrimitiveTopology::eTriangleFan;
		case ShaderPrimitiveTopology::TriangleListAdjacent:
			return vk::PrimitiveTopology::eTriangleListWithAdjacency;
		case ShaderPrimitiveTopology::TriangleStripAdjacent:
			return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
		default:
			LOG_ERROR("Failed to map shader primitive topology. Primitive topology used: %d", primitiveTopology);
	}
	return vk::PrimitiveTopology::eTriangleList;
}

// Gets the Vulkan polygon mode enum.
vk::PolygonMode mtd::PipelineMapping::mapPolygonMode(ShaderPrimitiveTopology primitiveTopology)
{
	switch(primitiveTopology)
	{
		case ShaderPrimitiveTopology::PointList:
			return vk::PolygonMode::ePoint;
		case ShaderPrimitiveTopology::LineList:
		case ShaderPrimitiveTopology::LineStrip:
		case ShaderPrimitiveTopology::LineListAdjacent:
		case ShaderPrimitiveTopology::LineStripAdjacent:
			return vk::PolygonMode::eLine;
		case ShaderPrimitiveTopology::TriangleList:
		case ShaderPrimitiveTopology::TriangleStrip:
		case ShaderPrimitiveTopology::TriangleFan:
		case ShaderPrimitiveTopology::TriangleListAdjacent:
		case ShaderPrimitiveTopology::TriangleStripAdjacent:
			return vk::PolygonMode::eFill;
		default:
			LOG_ERROR("Failed to map shader polygon mode. Primitive topology used: %d", primitiveTopology);
	}
	return vk::PolygonMode::eFill;
}

// Gets the Vulkan cull mode flags.
vk::CullModeFlags mtd::PipelineMapping::mapCullModeFlags(ShaderFaceCulling faceCulling)
{
	switch(faceCulling)
	{
		case ShaderFaceCulling::Clockwise:
		case ShaderFaceCulling::Counterclockwise:
			return vk::CullModeFlagBits::eBack;
		case ShaderFaceCulling::None:
			return vk::CullModeFlagBits::eNone;
		default:
			LOG_WARNING("Failed to map shader cull mode. Face culling enumeration used: %d", faceCulling);
	}
	return vk::CullModeFlagBits::eNone;
}

// Gets the Vulkan front face enum.
vk::FrontFace mtd::PipelineMapping::mapFrontFace(ShaderFaceCulling faceCulling)
{
	switch(faceCulling)
	{
		case ShaderFaceCulling::Clockwise:
			return vk::FrontFace::eClockwise;
		case ShaderFaceCulling::Counterclockwise:
		case ShaderFaceCulling::None:
			return vk::FrontFace::eCounterClockwise;
		default:
			LOG_WARNING("Failed to map shader front face. Face culling enumeration used: %d", faceCulling);
	}
	return vk::FrontFace::eCounterClockwise;
}
