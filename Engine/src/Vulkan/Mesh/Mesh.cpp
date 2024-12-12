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
	instanceBufferBindIndex{instanceBufferBindIndex},
	instanceBuffer
	{
		device,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	}
{
	for(const Mat4x4& preTransform: preTransforms)
	{
		std::unique_ptr<Model> pModel = modelFactory(preTransform);
		models[pModel->getInstanceID()] = std::move(pModel);
	}
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
}

// Runs once at the beginning of the scene for all instances
void mtd::Mesh::start()
{
	uint32_t i = 0;
	for(const auto& [instanceID, pModel]: models)
	{
		pModel->start();
		std::memcpy(&(instanceLump[i]), &(pModel->getTransform()), sizeof(Mat4x4));
		i++;
	}

	instanceBuffer.copyMemoryToBuffer(instanceLump.size() * sizeof(Mat4x4), instanceLump.data());
}

// Updates all instances
void mtd::Mesh::update(double deltaTime)
{
	if(models.size() <= 0) return;

	uint32_t i = 0;
	for(const auto& [instanceID, pModel]: models)
	{
		pModel->update(deltaTime);
		std::memcpy(&(instanceLump[i]), &(pModel->getTransform()), sizeof(Mat4x4));
		i++;
	}

	instanceBuffer.copyMemoryToBuffer(instanceLump.size() * sizeof(Mat4x4), instanceLump.data());
}

// Adds multiple new mesh instances with the identity pre-transform matrix
void mtd::Mesh::addInstances(const CommandHandler& commandHandler, uint32_t instanceCount)
{
	uint32_t minimumBufferSize = (models.size() + instanceCount) * sizeof(Mat4x4);
	if(minimumBufferSize > instanceBuffer.getSize())
	{
		uint32_t newSize = 2 * instanceBuffer.getSize();
		while(newSize < minimumBufferSize)
			newSize += newSize;

		instanceBuffer.resizeBuffer(commandHandler, newSize);
	}

	for(uint32_t i = 0; i < instanceCount; i++)
	{
		instanceLump.emplace_back(Mat4x4{1.0f});
		std::unique_ptr<Model> pModel = modelFactory(instanceLump.back());

		pModel->start();
		std::memcpy(&(instanceLump[i]), &(pModel->getTransform()), sizeof(Mat4x4));

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

	if(2 * expectedBufferSize <= instanceBuffer.getSize())
		instanceBuffer.resizeBuffer(commandHandler, expectedBufferSize);
}

// Creates a GPU buffer for the transformation matrices
void mtd::Mesh::createInstanceBuffer()
{
	instanceBuffer.create(instanceLump.size() * sizeof(Mat4x4));
	instanceBuffer.copyMemoryToBuffer(instanceBuffer.getSize(), instanceLump.data());
}

// Binds the instance buffer for this mesh
void mtd::Mesh::bindInstanceBuffer(const vk::CommandBuffer& commandBuffer) const
{
	vk::DeviceSize offset{0};
	commandBuffer.bindVertexBuffers(instanceBufferBindIndex, 1, &(instanceBuffer.getBuffer()), &offset);
}
