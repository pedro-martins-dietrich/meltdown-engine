#pragma once

#include "DefaultMesh.hpp"
#include "../BaseMeshManager.hpp"
#include "../../Device/GpuBuffer.hpp"

namespace mtd
{
	// Handles data from all default meshes
	class DefaultMeshManager : public BaseMeshManager<DefaultMesh>
	{
		public:
			DefaultMeshManager(const Device& device);
			~DefaultMeshManager();

			// Gets the total number of textures handled by the manager
			virtual uint32_t getTextureCount() const override { return static_cast<uint32_t>(meshes.size()); }

			// Loads textures and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) override;

			// Clears the list of default meshes and related buffers
			virtual void clearMeshes() override;

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

			// Total number of instances
			uint32_t totalInstanceCount;
			// Index offset counter
			uint32_t currentIndexOffset;

			// Stores a mesh in the lump of data
			void loadMeshToLump(DefaultMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
