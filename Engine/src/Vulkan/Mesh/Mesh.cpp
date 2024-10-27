#include <pch.hpp>
#include "Mesh.hpp"

#include <cstring>

#include "../../Utils/Logger.hpp"

mtd::Mesh::Mesh
(
	const Device& device,
	uint32_t index,
	const char* modelID,
	const Mat4x4& preTransform,
	uint32_t instanceBufferBindIndex
) : device{device},
	meshIndex{index},
	modelID{modelID},
	modelFactory{ModelHandler::getModelFactory(modelID)},
	models{},
	instanceBufferBindIndex{instanceBufferBindIndex}
{
	models.emplace_back(modelFactory(preTransform));
	instanceLump.emplace_back(preTransform);
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

// Adds a new instance
void mtd::Mesh::addInstance(const Mat4x4& preTransform)
{
	models.emplace_back(modelFactory(preTransform));
	instanceLump.emplace_back(preTransform);
}

// Creates a GPU buffer for the transformation matrices
void mtd::Mesh::createInstanceBuffer()
{
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
}

// Binds the instance buffer for this mesh
void mtd::Mesh::bindInstanceBuffer(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset{0};
	commandBuffer.bindVertexBuffers(instanceBufferBindIndex, 1, &(instanceBuffer.buffer), &offset);
}
