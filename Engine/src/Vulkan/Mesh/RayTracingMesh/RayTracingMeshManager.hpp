#pragma once

#include "RayTracingMesh.hpp"
#include "../BaseMeshManager.hpp"
#include "../../Pipeline/RayTracingPipeline.hpp"

namespace mtd
{
	class RayTracingMeshManager : public BaseMeshManager<RayTracingMesh>
	{
		public:
			RayTracingMeshManager(const Device& device, const MaterialInfo& materialInfo);
			~RayTracingMeshManager() = default;

			// Getters
			virtual uint32_t getMaterialCount() const override { return materialLump.getMaterialCount(); }
			virtual uint32_t getTextureCount() const override { return materialLump.getTextureCount(); }
			const GpuBuffer& getVertexBuffer() const { return vertexBuffer; }
			const GpuBuffer& getIndexBuffer() const { return indexBuffer; }

			// Checks if the material type for the stored meshes has float data
			virtual bool hasMaterialFloatData() const override { return materialLump.hasFloatData(); }

			// Loads the materials and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler) override;

			// Creates a new ray tracing mesh
			void createNewMesh
			(
				uint32_t index, const char* id, const char* file, const std::vector<Mat4x4>& preTransforms
			);

			// Binds vertex and index buffers
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override {}
			// Draws the meshes using a rasterization pipeline
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& graphicsPipeline
			) const override {}

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
			GpuBuffer materialIndexBuffer;

			// Lumps of data containing all vertices and indices from all meshes
			std::vector<Vertex> vertexLump;
			std::vector<uint32_t> indexLump;
			// Material index buffer data, with the material ID for each triangle
			std::vector<uint16_t> materialIndexLump;

			// Mesh manager materials
			MaterialLump materialLump;

			// Index offset counters
			uint32_t currentIndexOffset;
			uint32_t currentMaterialIndexOffset;

			// Stores a mesh in the lump of data
			void loadMeshToLump(RayTracingMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& traceRaysCommandHandler);
	};
}
