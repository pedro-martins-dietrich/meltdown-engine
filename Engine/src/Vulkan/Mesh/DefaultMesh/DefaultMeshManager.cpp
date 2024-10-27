#include <pch.hpp>
#include "DefaultMeshManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::DefaultMeshManager::DefaultMeshManager(const Device& device)
	: device{device}, currentIndexOffset{0}, totalInstanceCount{0}
{
}

mtd::DefaultMeshManager::~DefaultMeshManager()
{
	clearMeshes();
}

// Loads textures and groups the meshes into a lump, then passes the data to the GPU
void mtd::DefaultMeshManager::loadMeshes
(
	const CommandHandler& commandHandler, DescriptorSetHandler& textureDescriptorSetHandler
)
{
	for(DefaultMesh& mesh: meshes)
	{
		loadMeshToLump(mesh);
		mesh.loadTexture(device, commandHandler, textureDescriptorSetHandler);
	}
	loadMeshesToGPU(commandHandler);
}

// Clears the list of default meshes and related buffers
void mtd::DefaultMeshManager::clearMeshes()
{
	if(getMeshCount() == 0) return;

	const vk::Device& vulkanDevice = device.getDevice();
	vulkanDevice.waitIdle();

	totalInstanceCount = 0;
	meshes.clear();

	vulkanDevice.destroyBuffer(vertexBuffer.buffer);
	vulkanDevice.freeMemory(vertexBuffer.bufferMemory);

	vulkanDevice.destroyBuffer(indexBuffer.buffer);
	vulkanDevice.freeMemory(indexBuffer.bufferMemory);
}

// Executes the start code for each model on scene loading
void mtd::DefaultMeshManager::start()
{
	for(DefaultMesh& mesh: meshes)
		mesh.start();
}

// Updates instances data
void mtd::DefaultMeshManager::update(double frameTime)
{
	for(DefaultMesh& mesh: meshes)
		mesh.update(frameTime);
}

// Binds vertex and index buffers
void mtd::DefaultMeshManager::bindBuffers
(
	const vk::CommandBuffer& commandBuffer
) const
{
	vk::DeviceSize offset{0};
	commandBuffer.bindVertexBuffers(0, 1, &(vertexBuffer.buffer), &offset);
	commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

// Draws the mesh specified by the index
void mtd::DefaultMeshManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, uint32_t meshIndex
) const
{
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
	Memory::createDeviceLocalBuffer<Vertex>
	(
		device, vertexBuffer, vertexLump, vk::BufferUsageFlagBits::eVertexBuffer, commandHandler
	);

	Memory::createDeviceLocalBuffer<uint32_t>
	(
		device, indexBuffer, indexLump, vk::BufferUsageFlagBits::eIndexBuffer, commandHandler
	);

	for(DefaultMesh& defaultMesh: meshes)
		defaultMesh.createInstanceBuffer();

	vertexLump.clear();
	indexLump.clear();
	currentIndexOffset = 0;
}
