#pragma once

#include "MultiMaterial3DMesh.hpp"
#include "../BaseMeshManager.hpp"
#include "../../Device/GpuBuffer.hpp"

namespace mtd
{
	// Handles data from all multi-material 3D meshes
	class MultiMaterial3DMeshManager : public BaseMeshManager<MultiMaterial3DMesh>
	{
		public:
			MultiMaterial3DMeshManager(const Device& device);
			~MultiMaterial3DMeshManager() = default;

			// Gets the total number of textures handled by the manager
			virtual uint32_t getTextureCount() const override;

			// Loads textures and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) override;

			// Binds vertex and index buffers
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the mesh specified by the index
			virtual void drawMesh(const vk::CommandBuffer& commandBuffer, const Pipeline& pipeline) const override;

		private:
			// Vertex and index data of all meshes in the VRAM
			GpuBuffer vertexBuffer;
			GpuBuffer indexBuffer;

			// Lumps of data containing all vertices and indices from all meshes
			std::vector<Vertex> vertexLump;
			std::vector<uint32_t> indexLump;

			// Index offset counter
			uint32_t currentIndexOffset;

			// Stores a mesh in the lump of data
			void loadMeshToLump(MultiMaterial3DMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
