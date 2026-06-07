#include <pch.hpp>
#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../../Utils/Logger.hpp"

static constexpr std::array<uint32_t, 256> missingTexture = []() constexpr
{
	std::array<uint32_t, 256> texture{};
	for(size_t i = 0; i < 256; i++)
		texture[i] = (i % 2 ^ (i >> 4) % 2) ? 0xFF000000 : 0xFFFF00FF;
	return texture;
}();

mtd::Texture::Texture(const Device& mtdDevice, std::string_view fileName) : image{mtdDevice.getDevice()}
{
	loadFromFile(mtdDevice, fileName);
}

mtd::Texture::Texture(const Device& mtdDevice, std::string_view fileName, const CommandHandler& commandHandler)
	: image{mtdDevice.getDevice()}
{
	loadFromFile(mtdDevice, fileName);
	loadToGpu(mtdDevice, commandHandler);
}

mtd::Texture::Texture(const Device& mtdDevice, const CommandHandler& commandHandler) : image{mtdDevice.getDevice()}
{
	int width = 0;
	int height = 0;
	pixels = loadPlaceholderTexture(width, height);

	image.createImage
	(
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);
	image.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);

	loadToGpu(mtdDevice, commandHandler);
}

mtd::Texture::Texture
(
	const Device& mtdDevice,
	std::string_view fileName,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
	uint32_t binding
) : image{mtdDevice.getDevice()}
{
	loadFromFile(mtdDevice, fileName);
	loadToGpu(mtdDevice, commandHandler);
	createDescriptorResource(descriptorSetHandler, swappableSetIndex, binding);
}

mtd::Texture::Texture(Texture&& other) noexcept
	: channels{other.channels}, pixels{other.pixels}, image{std::move(other.image)},
	isPlaceholderTexture{other.isPlaceholderTexture}
{
	other.pixels = nullptr;
}

vk::DescriptorImageInfo mtd::Texture::getDescriptorImageInfo() const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.defineDescriptorImageInfo(&descriptorImageInfo);
	return descriptorImageInfo;
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

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();
	image.transitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);
	commandHandler.endSingleTimeCommand(commandBuffer);

	image.copyBufferToImage(commandHandler, stagingBuffer.getBuffer());

	commandBuffer = commandHandler.beginSingleTimeCommand();
	image.transitionImageLayout
	(
		commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eFragmentShader
	);
	commandHandler.endSingleTimeCommand(commandBuffer);

	if(!isPlaceholderTexture)
		free(pixels);

	image.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	image.createImageSampler(vk::Filter::eNearest);
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

	image.createImage
	(
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);
	image.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
}

stbi_uc* mtd::Texture::loadPlaceholderTexture(int& width, int& height)
{
	width = 16;
	height = 16;
	channels = 4;

	isPlaceholderTexture = true;
	return (stbi_uc*) missingTexture.data();
}

void mtd::Texture::createDescriptorResource
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.defineDescriptorImageInfo(&descriptorImageInfo);

	descriptorSetHandler.createImageDescriptorResources(swappableSetIndex, binding, descriptorImageInfo);
	descriptorSetHandler.writeDescriptorSet(swappableSetIndex);
}
