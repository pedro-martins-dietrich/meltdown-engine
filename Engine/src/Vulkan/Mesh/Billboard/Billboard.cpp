#include <pch.hpp>
#include "Billboard.hpp"

mtd::Billboard::Billboard
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* texturePath,
	const MaterialInfo& materialInfo,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 0}, material{materialInfo}
{
	material.addTexturePath(MaterialTextureType::DiffuseMap, texturePath);
}

mtd::Billboard::Billboard(Billboard&& other) noexcept
	: Mesh{std::move(other)},
	material{std::move(other.material)}
{
}

// Loads the billboard texture
void mtd::Billboard::loadTexture(const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler)
{
	material.loadMaterial(device, commandHandler, descriptorSetHandler, meshIndex);
}
