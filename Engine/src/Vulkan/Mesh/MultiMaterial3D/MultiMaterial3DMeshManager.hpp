#pragma once

#include "MultiMaterial3DMesh.hpp"
#include "../BaseMeshManager.hpp"

namespace mtd
{
	// Handles data from all multi-material 3D meshes
	class MultiMaterial3DMeshManager : public BaseMeshManager<MultiMaterial3DMesh>
	{
		public:
			MultiMaterial3DMeshManager(const Device& device);
			~MultiMaterial3DMeshManager() = default;

			// Getters
			virtual uint32_t getMaterialCount() const override;
			virtual uint32_t getTextureCount() const override;

			// Checks if the material type for the stored meshes has float data
			virtual bool hasMaterialFloatData() const override;

			// Loads the materials and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler) override;

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
			void loadMeshToLump(MultiMaterial3DMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
