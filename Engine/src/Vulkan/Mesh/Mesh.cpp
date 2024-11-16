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
	{
		std::unique_ptr<Model> pModel = modelFactory(preTransform);
		models[pModel->getInstanceID()] = std::move(pModel);
	}
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
	uint32_t i = 0;
	for(const auto& [instanceID, pModel]: models)
	{
		pModel->start();
		std::memcpy(&(instanceLump[i]), pModel->getTransformPointer(), sizeof(Mat4x4));
		i++;
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

	uint32_t i = 0;
	for(const auto& [instanceID, pModel]: models)
	{
		pModel->update(deltaTime);
		std::memcpy(&(instanceLump[i]), pModel->getTransformPointer(), sizeof(Mat4x4));
		i++;
	}

	Memory::copyMemory
	(
		device.getDevice(),
		instanceBuffer.bufferMemory,
		instanceLump.size() * sizeof(Mat4x4),
		instanceLump.data()
	);
}

// Adds multiple new mesh instances with the identity pre-transform matrix
void mtd::Mesh::addInstances(const CommandHandler& commandHandler, uint32_t instanceCount)
{
	uint32_t minimumBufferSize = (models.size() + instanceCount) * sizeof(Mat4x4);
	if(minimumBufferSize > instanceBuffer.size)
	{
		uint32_t newSize = 2 * instanceBuffer.size;
		while(newSize < minimumBufferSize)
			newSize += newSize;

		Memory::resizeBuffer(device, commandHandler, instanceBuffer, newSize);
	}

	for(uint32_t i = 0; i < instanceCount; i++)
	{
		instanceLump.emplace_back(Mat4x4{1.0f});
		std::unique_ptr<Model> pModel = modelFactory(instanceLump.back());

		pModel->start();
		std::memcpy(&(instanceLump[i]), pModel->getTransformPointer(), sizeof(Mat4x4));

		models[pModel->getInstanceID()] = std::move(pModel);
	}
}

// Removes the mesh instance associated with the provided instance ID
void mtd::Mesh::removeInstanceByID(const CommandHandler& commandHandler, uint64_t instanceID)
{
	if(models.find(instanceID) == models.cend()) return;

	models.erase(instanceID);
	instanceLump.pop_back();

	if(models.size() <= 0) return;

	vk::DeviceSize expectedBufferSize = models.size() * sizeof(Mat4x4);

	if(2 * expectedBufferSize <= instanceBuffer.size)
		Memory::resizeBuffer(device, commandHandler, instanceBuffer, expectedBufferSize);
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
