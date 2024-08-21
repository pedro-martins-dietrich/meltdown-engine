#include "Billboard.hpp"

mtd::Billboard::Billboard(uint32_t id, const char* texturePath, glm::mat4 preTransform)
	: Mesh{preTransform}, id{id}, texturePath{texturePath}
{
}

mtd::Billboard::Billboard(Billboard&& other) noexcept
	: Mesh{std::move(other.transforms), other.instanceLumpOffset, other.pInstanceLump},
	id{other.id},
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
		device, texturePath.c_str(), commandHandler, descriptorSetHandler, id
	);
}
