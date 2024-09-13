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
	instanceLump.clear();
	meshes.clear();

	vulkanDevice.destroyBuffer(instanceBuffer.buffer);
	vulkanDevice.freeMemory(instanceBuffer.bufferMemory);

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

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Updates instances data
void mtd::DefaultMeshManager::update(double frameTime)
{
	for(DefaultMesh& mesh: meshes)
		mesh.update(frameTime);

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Binds vertex and index buffers
void mtd::DefaultMeshManager::bindBuffers
(
	const vk::CommandBuffer& commandBuffer
) const
{
	std::array<vk::Buffer, 2> vertexBuffers{vertexBuffer.buffer, instanceBuffer.buffer};
	std::array<vk::DeviceSize, 2> offsets{0, 0};
	commandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

// Draws the mesh specified by the index
void mtd::DefaultMeshManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, uint32_t index
) const
{
	const DefaultMesh& mesh = meshes[index];
	commandBuffer.drawIndexed
	(
		static_cast<uint32_t>(mesh.getIndices().size()),
		mesh.getInstanceCount(),
		mesh.getIndexOffset(),
		0,
		mesh.getInstanceOffset()
	);
}

// Stores a mesh in the lump of data
void mtd::DefaultMeshManager::loadMeshToLump(DefaultMesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	mesh.setIndexOffset(static_cast<uint32_t>(indexLump.size()));

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	mesh.setInstancesLump(&instanceLump, instanceLump.size());

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

	vk::DeviceSize instanceLumpSize = instanceLump.size() * sizeof(Mat4x4);
	Memory::createBuffer
	(
		device,
		instanceBuffer,
		instanceLumpSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	Memory::copyMemory
	(
		device.getDevice(), instanceBuffer.bufferMemory, instanceLumpSize, instanceLump.data()
	);

	vertexLump.clear();
	indexLump.clear();
	currentIndexOffset = 0;
}
