#include "DefaultMeshManager.hpp"

mtd::DefaultMeshManager::DefaultMeshManager(const Device& device)
	: device{device}, currentIndexOffset{0}, totalInstanceCount{0}
{
}

mtd::DefaultMeshManager::~DefaultMeshManager()
{
	const vk::Device& vulkanDevice = device.getDevice();

	vulkanDevice.destroyBuffer(vertexBuffer.buffer);
	vulkanDevice.freeMemory(vertexBuffer.bufferMemory);

	vulkanDevice.destroyBuffer(indexBuffer.buffer);
	vulkanDevice.freeMemory(indexBuffer.bufferMemory);
}

// Stores a mesh in the lump of data
void mtd::DefaultMeshManager::loadMeshToLump(const Mesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	meshDrawInfos.emplace_back
	(
		static_cast<uint32_t>(indices.size()),
		mesh.getInstanceCount(),
		static_cast<uint32_t>(indexLump.size()),
		totalInstanceCount
	);

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	indexLump.reserve(indices.size());
	for(uint32_t index: indices)
	{
		indexLump.push_back(index + currentIndexOffset);
	}

	totalInstanceCount += mesh.getInstanceCount();
	currentIndexOffset += vertices.size();
}

// Loads the lumps into the VRAM and clears them
void mtd::DefaultMeshManager::loadMeshesToGPU(const CommandHandler& commandHandler)
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

// Binds vertex and index buffers
void mtd::DefaultMeshManager::bindBuffers
(
	const vk::CommandBuffer& commandBuffer
) const
{
	vk::DeviceSize offset = 0;
	commandBuffer.bindVertexBuffers(0, 1, &(vertexBuffer.buffer), &offset);
	commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

// Draws the mesh specified by the index
void mtd::DefaultMeshManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, uint32_t index
) const
{
	commandBuffer.drawIndexed
	(
		meshDrawInfos[index].indexCount,
		meshDrawInfos[index].instanceCount,
		meshDrawInfos[index].indexOffset,
		0,
		meshDrawInfos[index].startIndex
	);
}
