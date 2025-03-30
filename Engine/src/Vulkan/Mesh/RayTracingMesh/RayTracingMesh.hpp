#pragma once

#include "../Mesh.hpp"
#include "../../../Material/Material.hpp"

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
				const MaterialInfo& materialInfo,
				const std::vector<Mat4x4>& preTransforms
			);
			~RayTracingMesh() = default;

			RayTracingMesh(RayTracingMesh&& other) noexcept;

			// Getters
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			const std::vector<uint16_t>& getMaterialIndices() const { return materialIndices; }
			uint32_t getMaterialCount() const { return static_cast<uint32_t>(materials.size()); }
			uint32_t getTextureCount() const;

			// Checks if the used material has float data attributes
			bool hasMaterialFloatData() const;

			// Loads mesh materials
			void loadMaterials
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t initialMaterialIndex
			);

			// Deletes all mesh data on this object
			void clearMeshData();

		private:
			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<uint16_t> materialIndices;

			// Mesh materials
			std::vector<Material> materials;
	};
}
