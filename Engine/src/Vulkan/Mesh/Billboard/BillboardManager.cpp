#include <pch.hpp>
#include "BillboardManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : BaseMeshManager{device}
{
}

uint32_t mtd::BillboardManager::getTextureCount() const
{
	if(meshes.empty()) return 0;
	return static_cast<uint32_t>(meshes[0].getTextureCount() * meshes.size());
}

// Checks if the material type for the stored meshes has float data
bool mtd::BillboardManager::hasMaterialFloatData() const
{
	if(meshes.empty()) return false;
	return meshes[0].hasMaterialFloatData();
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

// There is no buffer common to all billboards to be binded
void mtd::BillboardManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& graphicsPipeline
) const
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		graphicsPipeline.bindMeshDescriptors(commandBuffer, i);

		const Billboard& billboard = meshes[i];
		billboard.bindInstanceBuffer(commandBuffer);
		commandBuffer.drawIndexed(6, billboard.getInstanceCount(), 0, 0, 0);
	}
}
