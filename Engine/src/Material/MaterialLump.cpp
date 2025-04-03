#include <pch.hpp>
#include "MaterialLump.hpp"

mtd::MaterialLump::MaterialLump(const Device& mtdDevice, const MaterialInfo& materialInfo)
	: materialInfo{materialInfo},
	floatDataBuffer{mtdDevice, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialCount{0U}
{}

// Loads material float data and textures
void mtd::MaterialLump::addMaterial(const float* data, size_t size)
{
	floatMaterialAttributes.insert(floatMaterialAttributes.cend(), data, data + size);
	materialCount++;
}

// Assigns the material buffer as a descriptor
void mtd::MaterialLump::assignBufferToDescriptor
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t bindingIndex
) const
{
	descriptorSetHandler.assignExternalResourcesToDescriptor(swappableSetIndex, bindingIndex, floatDataBuffer);
}

// Loads all materials to the GPU VRAM
void mtd::MaterialLump::loadMaterialsToGPU(const CommandHandler& commandHandler)
{
	floatDataBuffer.createDeviceLocal
	(
		commandHandler, sizeof(float) * floatMaterialAttributes.size(), floatMaterialAttributes.data()
	);
	floatMaterialAttributes.clear();
}
