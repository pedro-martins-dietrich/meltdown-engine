#include <pch.hpp>
#include "BillboardManager.hpp"

#include <meltdown/event.hpp>

#include "../../../Utils/Logger.hpp"

mtd::BillboardManager::BillboardManager(const Device& device) : device{device}
{
	EventManager::addCallback(EventType::ChangeInstanceCount, [this](const Event& e)
	{
		const ChangeInstanceCountEvent* pEvent = dynamic_cast<const ChangeInstanceCountEvent*>(&e);

		if(billboardIndexMap.find(pEvent->getModelID()) == billboardIndexMap.end()) return;

		Billboard& billboard = billboards[billboardIndexMap.at(pEvent->getModelID())];
		int32_t instanceVariation = pEvent->getInstanceCountVariation();

		if(instanceVariation < 0)
			billboard.removeLastInstances(static_cast<uint32_t>(-instanceVariation));
		else
		{
			billboard.addInstances(instanceVariation);
			billboard.startLastAddedInstances(instanceVariation);
		}
	});
}

mtd::BillboardManager::~BillboardManager()
{
	clearMeshes();
}

// Loads the billboards textures to the GPU
void mtd::BillboardManager::loadMeshes
(
	const CommandHandler& commandHandler, DescriptorSetHandler& textureDescriptorSetHandler
)
{
	for(uint32_t i = 0; i < billboards.size(); i++)
	{
		billboards[i].loadTexture(commandHandler, textureDescriptorSetHandler);
		billboards[i].createInstanceBuffer();
		billboardIndexMap[billboards[i].getModelID()] = i;
	}

	LOG_VERBOSE("Billboards loaded.");
}

// Clears the list of billboards and the instance buffer
void mtd::BillboardManager::clearMeshes()
{
	if(getMeshCount() == 0) return;

	const vk::Device& vulkanDevice = device.getDevice();
	vulkanDevice.waitIdle();

	billboards.clear();
}

// Executes the start code for each model on scene loading
void mtd::BillboardManager::start()
{
	for(Billboard& billboard: billboards)
		billboard.start();
}

// Updates instances data
void mtd::BillboardManager::update(double frameTime)
{
	for(Billboard& billboard: billboards)
		billboard.update(frameTime);
}

// There is no buffer common to all billboards to be binded
void mtd::BillboardManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
}

// Draws the mesh specified by the index
void mtd::BillboardManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, uint32_t meshIndex
) const
{
	const Billboard& billboard = billboards[meshIndex];
	billboard.bindInstanceBuffer(commandBuffer);
	commandBuffer.drawIndexed(6, billboard.getInstanceCount(), 0, 0, 0);
}
