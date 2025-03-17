#pragma once

#include "RayTracingMesh.hpp"
#include "../BaseMeshManager.hpp"
#include "../../Pipeline/RayTracingPipeline.hpp"

namespace mtd
{
	class RayTracingMeshManager : public BaseMeshManager<RayTracingMesh>
	{
		public:
			RayTracingMeshManager(const Device& device);
			~RayTracingMeshManager() = default;

			// Getters
			virtual uint32_t getMaterialCount() const override;
			virtual uint32_t getTextureCount() const override;
			const GpuBuffer& getVertexBuffer() const { return vertexBuffer; }
			const GpuBuffer& getIndexBuffer() const { return indexBuffer; }

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

			// Draws the meshes using a ray tracing pipeline
			void rayTraceMesh
			(
				const vk::CommandBuffer& commandBuffer,
				const RayTracingPipeline& rayTracingPipeline,
				const vk::detail::DispatchLoaderDynamic& dldi
			) const;

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
			void loadMeshToLump(RayTracingMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
