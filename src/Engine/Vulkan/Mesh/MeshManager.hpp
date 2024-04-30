#pragma once

#include "Mesh.hpp"
#include "../Device/Memory.hpp"

namespace mtd
{
	// Handles the vertices of multiple meshes
	class MeshManager
	{
		public:
			MeshManager(const Device& device);
			~MeshManager();

			MeshManager(const MeshManager&) = delete;
			MeshManager& operator=(const MeshManager&) = delete;

			// Getters
			const vk::Buffer& getVertexBuffer() const { return vertexBuffer.buffer; }
			const vk::Buffer& getIndexBuffer() const { return indexBuffer.buffer; }
			const std::vector<uint32_t>& getIndexCounts() const { return indexCounts; }
			const std::vector<uint32_t>& getInstanceCounts() const { return instanceCounts; }
			const std::vector<uint32_t>& getIndexOffsets() const { return indexOffsets; }
			vk::DeviceSize getModelMatricesSize() const
				{ return sizeof(glm::mat4) * totalInstanceCount; }

			// Stores a mesh in the lump of data
			void loadMeshToLump(const Mesh& mesh);
			// Loads the lumps into the VRAM and clears them
			void loadMeshesToGPU(const CommandHandler& commandHandler);

		private:
			// Vertex and index data of all meshes in the VRAM
			Memory::Buffer vertexBuffer;
			Memory::Buffer indexBuffer;

			// Index and instance data for each mesh ID
			std::vector<uint32_t> indexCounts;
			std::vector<uint32_t> instanceCounts;
			std::vector<uint32_t> indexOffsets;

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
