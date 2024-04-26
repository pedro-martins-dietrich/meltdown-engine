#include "MeshManager.hpp"

mtd::MeshManager::MeshManager(const Device& device)
	: device{device}, currentIndexOffset{0}
{
}

mtd::MeshManager::~MeshManager()
{
	const vk::Device& vulkanDevice = device.getDevice();

	vulkanDevice.destroyBuffer(vertexBuffer.buffer);
	vulkanDevice.freeMemory(vertexBuffer.bufferMemory);

	vulkanDevice.destroyBuffer(indexBuffer.buffer);
	vulkanDevice.freeMemory(indexBuffer.bufferMemory);
}

// Stores a mesh in the lump of data
void mtd::MeshManager::loadMeshToLump(const Mesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	indexCounts.push_back(static_cast<uint32_t>(indices.size()));
	instanceCounts.push_back(mesh.getInstanceCount());
	indexOffsets.push_back(static_cast<uint32_t>(indexLump.size()));

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	indexLump.reserve(indices.size());
	for(uint32_t index: indices)
	{
		indexLump.push_back(index + currentIndexOffset);
	}

	currentIndexOffset += vertices.size();
}

// Loads the lumps into the VRAM and clears them
void mtd::MeshManager::loadMeshesToGPU(const CommandHandler& commandHandler)
{
	Memory::Buffer stagingBuffer;

	// Vertex loading
	vk::DeviceSize vertexLumpSize = vertexLump.size() * sizeof(Vertex);
	Memory::createBuffer
	(
		device,
		stagingBuffer,
		vertexLumpSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	Memory::copyMemory
	(
		device.getDevice(), stagingBuffer.bufferMemory, vertexLumpSize, vertexLump.data()
	);

	Memory::createBuffer
	(
		device,
		vertexBuffer,
		vertexLumpSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	Memory::copyBuffer(stagingBuffer, vertexBuffer, vertexLumpSize, commandHandler);

	device.getDevice().destroyBuffer(stagingBuffer.buffer);
	device.getDevice().freeMemory(stagingBuffer.bufferMemory);

	// Index loading
	vk::DeviceSize indexLumpSize = indexLump.size() * sizeof(uint32_t);
	Memory::createBuffer
	(
		device,
		stagingBuffer,
		indexLumpSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	Memory::copyMemory
	(
		device.getDevice(), stagingBuffer.bufferMemory, indexLumpSize, indexLump.data()
	);
	Memory::createBuffer
	(
		device,
		indexBuffer,
		indexLumpSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	Memory::copyBuffer(stagingBuffer, indexBuffer, indexLumpSize, commandHandler);

	device.getDevice().destroyBuffer(stagingBuffer.buffer);
	device.getDevice().freeMemory(stagingBuffer.bufferMemory);

	vertexLump.clear();
	indexLump.clear();
	currentIndexOffset = 0;
}
