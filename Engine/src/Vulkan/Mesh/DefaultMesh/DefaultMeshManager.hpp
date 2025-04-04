#pragma once

#include "DefaultMesh.hpp"
#include "../BaseMeshManager.hpp"

namespace mtd
{
	// Handles data from all default meshes
	class DefaultMeshManager : public BaseMeshManager<DefaultMesh>
	{
		public:
			DefaultMeshManager(const Device& device);
			~DefaultMeshManager() = default;

			// Getters
			virtual uint32_t getMaterialCount() const override { return static_cast<uint32_t>(meshes.size()); }
			virtual uint32_t getTextureCount() const override;

			// Checks if the material type for the stored meshes has float data
			virtual bool hasMaterialFloatData() const override;

			// Loads textures and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) override;

			// Binds vertex and index buffers
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;

			// Draws the meshes using a rasterization pipeline
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& graphicsPipeline
			) const override;

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
			void loadMeshToLump(DefaultMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
