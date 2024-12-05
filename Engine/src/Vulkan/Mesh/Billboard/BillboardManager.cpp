#include <pch.hpp>
#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : BaseMeshManager{device}
{
}

mtd::BillboardManager::~BillboardManager()
{
	clearMeshes();
}

// Loads the billboards textures to the GPU
void mtd::BillboardManager::loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler)
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		meshes[i].loadTexture(commandHandler, textureDescriptorSetHandler);
		meshes[i].createInstanceBuffer();
		meshIndexMap[meshes[i].getModelID()] = i;
	}

	LOG_VERBOSE("Billboards loaded.");
}

// Clears the list of billboards and the instance buffer
void mtd::BillboardManager::clearMeshes()
{
	if(getMeshCount() == 0) return;

	const vk::Device& vulkanDevice = device.getDevice();
	vulkanDevice.waitIdle();

	meshes.clear();
}

// There is no buffer common to all billboards to be binded
void mtd::BillboardManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh(const vk::CommandBuffer& commandBuffer, const Pipeline& pipeline) const
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		pipeline.bindMeshDescriptors(commandBuffer, i);

		const Billboard& billboard = meshes[i];
		billboard.bindInstanceBuffer(commandBuffer);
		commandBuffer.drawIndexed(6, billboard.getInstanceCount(), 0, 0, 0);
	}
}
