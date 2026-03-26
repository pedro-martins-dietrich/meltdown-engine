#pragma once

#include <cstdint>

namespace mtd
{
	struct MeshHeader
	{
		uint64_t magic;
		uint32_t version;
		uint32_t vertexType;
		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t submeshCount;
		uint32_t vertexDataOffset;
		uint32_t indexDataOffset;
		uint32_t submeshDataOffset;
		uint32_t flags;
	};

	struct MaterialHeader
	{
		uint64_t magic;
		uint32_t version;
		uint32_t floatDataTypeCount;
		uint32_t textureCount;
		uint32_t floatDataCount;
		uint32_t floatDataTypesOffset;
		uint32_t textureTypesOffset;
		uint32_t floatDataOffset;
		uint32_t texturePathsOffset;
		uint32_t flags;
	};
}
