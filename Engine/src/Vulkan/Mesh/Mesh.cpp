#include <pch.hpp>
#include "Mesh.hpp"

#include <cstring>

mtd::Mesh::Mesh
(
	const Device& device,
	uint32_t index,
	const char* modelID,
	const std::vector<Mat4x4>& preTransforms,
	uint32_t instanceBufferBindIndex
) : device{device},
	meshIndex{index},
	modelID{modelID},
	modelFactory{ModelHandler::getModelFactory(modelID)},
	models{},
	instanceLump{preTransforms},
	instanceBufferBindIndex{instanceBufferBindIndex}
{

	for(const Mat4x4& preTransform: preTransforms)
		models.emplace_back(modelFactory(preTransform));
}

mtd::Mesh::~Mesh()
{
	device.getDevice().destroyBuffer(instanceBuffer.buffer);
	device.getDevice().freeMemory(instanceBuffer.bufferMemory);
}

mtd::Mesh::Mesh(Mesh&& other) noexcept
	: device{other.device},
	meshIndex{other.meshIndex},
	modelID{std::move(other.modelID)},
	modelFactory{std::move(other.modelFactory)},
	models{std::move(other.models)},
	instanceLump{std::move(other.instanceLump)},
	instanceBuffer{std::move(other.instanceBuffer)},
	instanceBufferBindIndex{other.instanceBufferBindIndex}
{
	other.instanceBuffer.buffer = nullptr;
	other.instanceBuffer.bufferMemory = nullptr;
}

// Runs once at the beginning of the scene for all instances
void mtd::Mesh::start()
{
	for(uint32_t instanceIndex = 0; instanceIndex < models.size(); instanceIndex++)
	{
		models[instanceIndex]->start();
		std::memcpy
		(
			&(instanceLump[instanceIndex]),
			models[instanceIndex]->getTransformPointer(),
			sizeof(Mat4x4)
		);
	}

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Updates all instances
void mtd::Mesh::update(double deltaTime)
{
	if(models.size() <= 0) return;

	for(uint32_t instanceIndex = 0; instanceIndex < models.size(); instanceIndex++)
	{
		models[instanceIndex]->update(deltaTime);
		std::memcpy
		(
			&(instanceLump[instanceIndex]),
			models[instanceIndex]->getTransformPointer(),
			sizeof(Mat4x4)
		);
	}

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Starts last instance added
void mtd::Mesh::startLastAddedInstances(uint32_t instanceCount)
{
	for(uint32_t i = models.size() - instanceCount; i < models.size(); i++)
	{
		models[i]->start();
		std::memcpy(&(instanceLump[i]), models[i]->getTransformPointer(), sizeof(Mat4x4));
	}
}

// Adds multiple new mesh instances with the identity pre-transform matrix
void mtd::Mesh::addInstances(uint32_t instanceCount)
{
	uint32_t minimumBufferSize = (models.size() + instanceCount) * sizeof(Mat4x4);
	if(minimumBufferSize > instanceBuffer.size)
	{
		uint32_t newSize = 2 * instanceBuffer.size;
		while(newSize < minimumBufferSize)
			newSize += newSize;

		Memory::resizeBuffer(device, instanceBuffer, newSize);
	}

	for(uint32_t i = 0; i < instanceCount; i++)
	{
		models.emplace_back(modelFactory(Mat4x4{1.0f}));
		instanceLump.emplace_back(Mat4x4{1.0f});
	}
}

// Removes the last mesh instances
void mtd::Mesh::removeLastInstances(uint32_t instanceCount)
{
	if(models.size() <= 0) return;
	if(models.size() < instanceCount)
		instanceCount = models.size();

	models.erase(models.cend() - instanceCount, models.cend());
	instanceLump.erase(instanceLump.cend() - instanceCount, instanceLump.cend());

	if(models.size() <= 0) return;

	uint32_t newBufferSizeMaxLimit = 2 * models.size() * sizeof(Mat4x4);
	if(newBufferSizeMaxLimit <= instanceBuffer.size)
	{
		uint32_t newSize = instanceBuffer.size / 2;
		while(newBufferSizeMaxLimit <= newSize)
			newSize >>= 1;

		Memory::resizeBuffer(device, instanceBuffer, newSize);
	}
}

// Creates a GPU buffer for the transformation matrices
void mtd::Mesh::createInstanceBuffer()
{
	instanceBuffer.size = instanceLump.size() * sizeof(Mat4x4);
	instanceBuffer.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc;
	instanceBuffer.memoryProperties =
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

	Memory::createBuffer(device, instanceBuffer);
	Memory::copyMemory
	(
		device.getDevice(), instanceBuffer.bufferMemory, instanceBuffer.size, instanceLump.data()
	);
}

// Binds the instance buffer for this mesh
void mtd::Mesh::bindInstanceBuffer(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset{0};
	commandBuffer.bindVertexBuffers(instanceBufferBindIndex, 1, &(instanceBuffer.buffer), &offset);
}
