#include <pch.hpp>
#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../../Utils/Logger.hpp"

mtd::Texture::Texture(const Device& mtdDevice, const char* fileName)
	: width{0}, height{0}, channels{0}, pixels{nullptr}, image{mtdDevice.getDevice()}
{
	loadFromFile(mtdDevice, fileName);
}

mtd::Texture::Texture
(
	const Device& mtdDevice,
	const char* fileName,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
	uint32_t binding
) : width{0}, height{0}, channels{0}, pixels{nullptr}, image{mtdDevice.getDevice()}
{
	loadFromFile(mtdDevice, fileName);
	loadToGpu(mtdDevice, commandHandler);
	createDescriptorResource(descriptorSetHandler, swappableSetIndex, binding);
}

mtd::Texture::Texture(Texture&& other) noexcept
	: width{other.width}, height{other.height}, channels{other.channels},
	pixels{other.pixels},
	image{std::move(other.image)}
{
	other.pixels = nullptr;
}

vk::DescriptorImageInfo mtd::Texture::getDescriptorImageInfo() const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	image.defineDescriptorImageInfo(&descriptorImageInfo);
	return descriptorImageInfo;
}

// Sends the texture data to the GPU
void mtd::Texture::loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler)
{
	assert(pixels != nullptr && "The image must be loaded from file before loading to the GPU.");

	const vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(width * height * 4);
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
	image.transitionImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
	commandHandler.endSingleTimeCommand(commandBuffer);

	free(pixels);
	image.createImageView(vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D);
	image.createImageSampler(vk::Filter::eNearest);
}

// Loads texture from file
void mtd::Texture::loadFromFile(const Device& mtdDevice, const char* fileName)
{
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(fileName, &width, &height, &channels, STBI_rgb_alpha);
	if(!pixels)
		LOG_ERROR("Failed to load texture \"%s\".", fileName);

	image.createImage
	(
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);
	image.createImageMemory(mtdDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
}

// Configures the texture descriptor set
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
