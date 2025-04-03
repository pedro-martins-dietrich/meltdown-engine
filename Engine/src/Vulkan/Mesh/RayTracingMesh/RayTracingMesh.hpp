#pragma once

#include "../Mesh.hpp"
#include "../../../Material/MaterialLump.hpp"

namespace mtd
{
	// Stores mesh data for ray tracing based rendering
	class RayTracingMesh : public Mesh
	{
		public:
			RayTracingMesh
			(
				const Device& device,
				uint32_t index,
				const char* id,
				const char* fileName,
				MaterialLump& materialLump,
				const std::vector<Mat4x4>& preTransforms
			);
			~RayTracingMesh() = default;

			RayTracingMesh(RayTracingMesh&& other) noexcept;

			// Getters
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			const std::vector<uint16_t>& getMaterialIndices() const { return materialIndices; }
			uint32_t getMaterialCount() const { return materialCount; }

			// Deletes all mesh data on this object
			void clearMeshData();

		private:
			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<uint16_t> materialIndices;

			// Mesh material count
			uint32_t materialCount;
	};
}
