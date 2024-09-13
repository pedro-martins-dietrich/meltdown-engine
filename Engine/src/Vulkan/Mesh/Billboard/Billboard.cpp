#include <pch.hpp>
#include "Billboard.hpp"

mtd::Billboard::Billboard
(
	uint32_t index, const char* id, const char* texturePath, const Mat4x4& preTransform
) : Mesh{index, id, preTransform}, texturePath{texturePath}
{
}

mtd::Billboard::Billboard(Billboard&& other) noexcept
	: Mesh
	{
		other.meshIndex,
		other.modelID,
		std::move(other.models),
		other.instanceLumpOffset,
		other.pInstanceLump
	},
	texturePath{other.texturePath},
	texture{std::move(other.texture)}
{
	other.texture = nullptr;
	other.pInstanceLump = nullptr;
}

// Loads the billboard texture
void mtd::Billboard::loadTexture
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler
)
{
	texture = std::make_unique<Texture>
	(
		device, texturePath.c_str(), commandHandler, descriptorSetHandler, meshIndex
	);
}
