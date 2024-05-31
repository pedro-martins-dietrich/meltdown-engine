#pragma once

#include "MeshManager.hpp"
#include "../Mesh.hpp"
#include "../../Device/Memory.hpp"

namespace mtd
{
	// Index and instance data for drawing a single mesh
	struct MeshDrawData
	{
		uint32_t indexCount;
		uint32_t instanceCount;
		uint32_t indexOffset;
		uint32_t startIndex;
	};

	// Handles the vertices of multiple default meshes
	class DefaultMeshManager : public MeshManager
	{
		public:
			DefaultMeshManager(const Device& device);
			~DefaultMeshManager();

			DefaultMeshManager(const DefaultMeshManager&) = delete;
			DefaultMeshManager& operator=(const DefaultMeshManager&) = delete;

			// Getters
			virtual uint32_t getMeshCount() const override
				{ return static_cast<uint32_t>(meshDrawInfos.size()); }
			uint32_t getTotalInstanceCount() const { return totalInstanceCount; }

			// Stores a mesh in the lump of data
			void loadMeshToLump(const Mesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);

			// Binds vertex and index buffers
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the mesh specified by the index
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, uint32_t index
			) const override;

		private:
			// Vertex and index data of all meshes in the VRAM
			Memory::Buffer vertexBuffer;
			Memory::Buffer indexBuffer;

			// Index and instance data for each mesh ID
			std::vector<MeshDrawData> meshDrawInfos;

			// Lumps of data containing all vertices and indices from all meshes
			std::vector<Vertex> vertexLump;
			std::vector<uint32_t> indexLump;
			// Total number of instances
			uint32_t totalInstanceCount;

			// Index offset counter
			uint32_t currentIndexOffset;

			// Device reference
			const Device& device;
	};
}
