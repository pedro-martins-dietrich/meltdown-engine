#pragma once

#include "../../Utils/EngineStructs.hpp"

// Bottom level acceleration structure builder for compute and rasterization pipelines
namespace mtd::BlasBuilder
{
	// Builds the BLAS for a single mesh
	bool buildBLAS
	(
		const MeshData& mesh, const std::vector<std::byte>& vertices, const std::vector<uint32_t>& indices,
		std::vector<std::byte>& blasData
	);
}
