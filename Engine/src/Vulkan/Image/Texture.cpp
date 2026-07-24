#include <pch.hpp>
#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../../Utils/Logger.hpp"

static constexpr std::array<uint32_t, 256> missingTexture = []() constexpr
{
	std::array<uint32_t, 256> texture{};
	for(size_t i = 0UL; i < 256UL; i++)
		texture[i] = (i % 2 ^ (i >> 4) % 2) ? 0xFF000000 : 0xFFFF00FF;
	return texture;
}();

mtd::Texture::Texture(const Device& mtdDevice, std::string_view fileName) : image{mtdDevice}
{
	loadFromFile(mtdDevice, fileName);
}

mtd::Texture::Texture(const Device& mtdDevice, std::string_view fileName, const CommandHandler& commandHandler)
	: image{mtdDevice}
{
	loadFromFile(mtdDevice, fileName);
	loadToGpu(mtdDevice, commandHandler);
}

mtd::Texture::Texture(const Device& mtdDevice, const CommandHandler& commandHandler) : image{mtdDevice}
{
	int width = 0;
	int height = 0;
	pixels = loadPlaceholderTexture(width, height);

	image.create
	(
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		SamplerType::Linear
	);

	loadToGpu(mtdDevice, commandHandler);
}

mtd::Texture::Texture
(
	const Device& mtdDevice,
	std::string_view fileName,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t binding
) : image{mtdDevice}
{
	loadFromFile(mtdDevice, fileName);
	loadToGpu(mtdDevice, commandHandler);
	createDescriptorResource(descriptorSetHandler, binding);
}

mtd::Texture::Texture(Texture&& other) noexcept
	: channels{other.channels}, pixels{other.pixels}, image{std::move(other.image)},
	isPlaceholderTexture{other.isPlaceholderTexture}
{
	other.pixels = nullptr;
}

void mtd::Texture::loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler)
{
	assert(pixels != nullptr && "The image must be loaded from file before loading to the GPU.");

	UIntVec2 dimensions = image.getDimensions();
	const vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(dimensions.x * dimensions.y * 4UL);

	GpuBuffer stagingBuffer
	{
		mtdDevice,
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
	};
	stagingBuffer.copyMemoryToBuffer(imageSize, pixels);

	image.copyBufferToImage(commandHandler, stagingBuffer.getBuffer());

	if(!isPlaceholderTexture)
		free(pixels);
}

void mtd::Texture::loadFromFile(const Device& mtdDevice, std::string_view fileName)
{
	int width = 0;
	int height = 0;

	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(fileName.data(), &width, &height, &channels, STBI_rgb_alpha);
	if(!pixels)
	{
		LOG_WARNING("Failed to load texture \"%s\".", fileName.data());
		pixels = loadPlaceholderTexture(width, height);
	}

	image.create
	(
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		SamplerType::Linear
	);
}

stbi_uc* mtd::Texture::loadPlaceholderTexture(int& width, int& height)
{
	width = 16;
	height = 16;
	channels = 4;

	isPlaceholderTexture = true;
	return (stbi_uc*) missingTexture.data();
}

void mtd::Texture::createDescriptorResource(DescriptorSetHandler& descriptorSetHandler, uint32_t binding) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.updateDescriptorInfo(descriptorImageInfo);

	descriptorSetHandler.createImageDescriptorResources(binding, descriptorImageInfo);
	descriptorSetHandler.writeDescriptor(binding);
}
