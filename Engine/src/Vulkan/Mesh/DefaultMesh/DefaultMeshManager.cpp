#include <pch.hpp>
#include "DefaultMeshManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::DefaultMeshManager::DefaultMeshManager(const Device& device)
	: BaseMeshManager{device},
	currentIndexOffset{0},
	totalInstanceCount{0},
	vertexBuffer{device, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	indexBuffer{device, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
{
}

mtd::DefaultMeshManager::~DefaultMeshManager()
{
	clearMeshes();
}

// Loads textures and groups the meshes into a lump, then passes the data to the GPU
void mtd::DefaultMeshManager::loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler)
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);
		meshes[i].loadTexture(device, commandHandler, textureDescriptorSetHandler);
		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);
}

// Clears the list of default meshes and related buffers
void mtd::DefaultMeshManager::clearMeshes()
{
	if(getMeshCount() == 0) return;

	device.getDevice().waitIdle();

	totalInstanceCount = 0;
	meshes.clear();
}

// Binds vertex and index buffers
void mtd::DefaultMeshManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset{0};
	commandBuffer.bindVertexBuffers(0, 1, &(vertexBuffer.getBuffer()), &offset);
	commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);
}

// Draws the mesh specified by the index
void mtd::DefaultMeshManager::drawMesh(const vk::CommandBuffer& commandBuffer, const Pipeline& pipeline) const
{
	for(uint32_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
	{
		pipeline.bindMeshDescriptors(commandBuffer, meshIndex);

		const DefaultMesh& mesh = meshes[meshIndex];
		mesh.bindInstanceBuffer(commandBuffer);
		commandBuffer.drawIndexed
		(
			static_cast<uint32_t>(mesh.getIndices().size()),
			mesh.getInstanceCount(),
			mesh.getIndexOffset(),
			0,
			0
		);
	}
}

// Stores a mesh in the lump of data
void mtd::DefaultMeshManager::loadMeshToLump(DefaultMesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	mesh.setIndexOffset(static_cast<uint32_t>(indexLump.size()));

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	indexLump.reserve(indices.size());
	for(uint32_t index: indices)
		indexLump.push_back(index + currentIndexOffset);

	totalInstanceCount += mesh.getInstanceCount();
	currentIndexOffset += vertices.size();
}

// Loads the lumps into the VRAM and clears them
void mtd::DefaultMeshManager::loadMeshesToGPU(const CommandHandler& commandHandler)
{
	vertexBuffer.createDeviceLocal(commandHandler, sizeof(Vertex) * vertexLump.size(), vertexLump.data());
	indexBuffer.createDeviceLocal(commandHandler, sizeof(uint32_t) * indexLump.size(), indexLump.data());

	for(DefaultMesh& defaultMesh: meshes)
		defaultMesh.createInstanceBuffer();

	vertexLump.clear();
	indexLump.clear();
	currentIndexOffset = 0;
}
