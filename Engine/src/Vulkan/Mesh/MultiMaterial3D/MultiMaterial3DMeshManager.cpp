#include <pch.hpp>
#include "MultiMaterial3DMeshManager.hpp"

mtd::MultiMaterial3DMeshManager::MultiMaterial3DMeshManager(const Device& device)
	: BaseMeshManager{device},
	currentIndexOffset{0},
	vertexBuffer{device, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	indexBuffer{device, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
{
}

// Gets the total number of textures handled by the manager
uint32_t mtd::MultiMaterial3DMeshManager::getTextureCount() const
{
	uint32_t totalTextureCount = 0;
	for(const MultiMaterial3DMesh& mesh: meshes)
		totalTextureCount += mesh.getTextureCount();
	return totalTextureCount;
}

// Loads textures and groups the meshes into a lump, then passes the data to the GPU
void mtd::MultiMaterial3DMeshManager::loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler)
{
	uint32_t currentTextureCount = 0;
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);

		meshes[i].loadMaterials(device, commandHandler, textureDescriptorSetHandler, currentTextureCount);
		currentTextureCount += meshes[i].getTextureCount();

		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);
}

// Binds vertex and index buffers
void mtd::MultiMaterial3DMeshManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &(vertexBuffer.getBuffer()), &offset);
	commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);
}

// Draws the mesh specified by the index
void mtd::MultiMaterial3DMeshManager::drawMesh(const vk::CommandBuffer& commandBuffer, const Pipeline& pipeline) const
{
	uint32_t materialOffset = 0;
	for(uint32_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
	{
		const MultiMaterial3DMesh& mesh = meshes[meshIndex];
		mesh.bindInstanceBuffer(commandBuffer);
		for(uint32_t submeshIndex = 0; submeshIndex < mesh.getSubmeshCount(); submeshIndex++)
		{
			pipeline.bindMeshDescriptors(commandBuffer, materialOffset + mesh.getSubmeshMaterialIndex(submeshIndex));
			commandBuffer.drawIndexed
			(
				mesh.getSubmeshIndexCount(submeshIndex),
				mesh.getInstanceCount(),
				mesh.getSubmeshIndexOffset(submeshIndex),
				0,
				0
			);
		}
		materialOffset += mesh.getTextureCount();
	}
}

// Stores a mesh in the lump of data
void mtd::MultiMaterial3DMeshManager::loadMeshToLump(MultiMaterial3DMesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	mesh.setIndexOffset(static_cast<uint32_t>(indexLump.size()));

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	indexLump.reserve(indices.size());
	for(uint32_t index: indices)
		indexLump.push_back(index + currentIndexOffset);

	currentIndexOffset += vertices.size();
}

// Loads the lumps into the VRAM and clears them
void mtd::MultiMaterial3DMeshManager::loadMeshesToGPU(const CommandHandler& commandHandler)
{
	vertexBuffer.createDeviceLocal(commandHandler, sizeof(Vertex) * vertexLump.size(), vertexLump.data());
	indexBuffer.createDeviceLocal(commandHandler, sizeof(uint32_t) * indexLump.size(), indexLump.data());

	for(MultiMaterial3DMesh& mesh: meshes)
		mesh.createInstanceBuffer();

	vertexLump.clear();
	indexLump.clear();
	currentIndexOffset = 0;
}
