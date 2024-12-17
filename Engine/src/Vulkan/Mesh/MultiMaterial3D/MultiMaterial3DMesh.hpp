#pragma once

#include "../Mesh.hpp"
#include "../../../Material/Material.hpp"

namespace mtd
{
	// Stores data for 3D meshes with more than a single material
	class MultiMaterial3DMesh : public Mesh
	{
		public:
			MultiMaterial3DMesh
			(
				const Device& device,
				uint32_t index,
				const char* id,
				const char* fileName,
				const MaterialInfo& materialInfo,
				const std::vector<Mat4x4>& preTransforms
			);
			~MultiMaterial3DMesh() = default;

			MultiMaterial3DMesh(MultiMaterial3DMesh&& other) noexcept;

			// Getters
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			uint32_t getMaterialCount() const { return static_cast<uint32_t>(materials.size()); }
			uint32_t getTextureCount() const;
			uint32_t getSubmeshCount() const { return static_cast<uint32_t>(submeshInfos.size()); }
			uint32_t getSubmeshIndexOffset(uint32_t submeshIndex) const
				{ return submeshInfos[submeshIndex].indexOffset; }
			uint32_t getSubmeshMaterialIndex(uint32_t submeshIndex) const
				{ return submeshInfos[submeshIndex].materialID; }
			uint32_t getSubmeshIndexCount(uint32_t submeshIndex) const;

			// Sets all sub-mesh index offsets in the lump
			void setIndexOffset(uint32_t offset);

			// Checks if the used material has float data attributes
			bool hasMaterialFloatData() const;

			// Loads mesh materials
			void loadMaterials
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t initialTextureIndex
			);

		private:
			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<SubmeshData> submeshInfos;
			uint32_t nextMeshIndexOffset;

			// Mesh materials
			std::vector<Material> materials;
	};
}
