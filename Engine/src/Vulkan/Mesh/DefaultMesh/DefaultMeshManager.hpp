#pragma once

#include "DefaultMesh.hpp"
#include "../MeshManager.hpp"
#include "../../Device/Memory.hpp"

namespace mtd
{
	// Handles data from all default meshes
	class DefaultMeshManager : public MeshManager
	{
		public:
			DefaultMeshManager(const Device& device);
			~DefaultMeshManager();

			DefaultMeshManager(const DefaultMeshManager&) = delete;
			DefaultMeshManager& operator=(const DefaultMeshManager&) = delete;

			// Getters
			virtual uint32_t getMeshCount() const override
				{ return static_cast<uint32_t>(meshes.size()); }
			uint32_t getTotalInstanceCount() const { return totalInstanceCount; }
			std::vector<DefaultMesh>& getMeshes() { return meshes; }

			// Loads textures and groups the meshes into a lump, then passes the data to the GPU
			virtual void loadMeshes
			(
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			) override;

			// Clears the list of default meshes and related buffers
			virtual void clearMeshes() override;

			// Executes the start code for each model on scene loading
			virtual void start() override;
			// Updates instances data
			virtual void update(double frameTime) override;

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
			// Transformation matrices for each instance
			Memory::Buffer instanceBuffer;

			// Default meshes
			std::vector<DefaultMesh> meshes;

			// Lumps of data containing all vertices and indices from all meshes
			std::vector<Vertex> vertexLump;
			std::vector<uint32_t> indexLump;
			std::vector<Mat4x4> instanceLump;

			// Total number of instances
			uint32_t totalInstanceCount;
			// Index offset counter
			uint32_t currentIndexOffset;

			// Device reference
			const Device& device;

			// Stores a mesh in the lump of data
			void loadMeshToLump(DefaultMesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);
	};
}
