#include <pch.hpp>
#include "Billboard.hpp"

mtd::Billboard::Billboard
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* texturePath,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 0}, texturePath{texturePath}
{
}

mtd::Billboard::Billboard(Billboard&& other) noexcept
	: Mesh{std::move(other)},
	texturePath{other.texturePath},
	texture{std::move(other.texture)}
{
	other.texture = nullptr;
}

// Loads the billboard texture
void mtd::Billboard::loadTexture(const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler)
{
	texture = std::make_unique<Texture>
	(
		device, texturePath.c_str(), commandHandler, descriptorSetHandler, meshIndex
	);
}
