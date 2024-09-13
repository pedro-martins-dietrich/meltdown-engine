#include <pch.hpp>
#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Image.hpp"
#include "../../Utils/Logger.hpp"
#include "../../Utils/FileHandler.hpp"

mtd::Texture::Texture
(
	const Device& mtdDevice,
	const char* fileName,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t setIndex
) : device{mtdDevice.getDevice()}, width{0}, height{0}, channels{0}
{
	loadFromFile(mtdDevice, commandHandler, fileName);
	createDescriptorResource(descriptorSetHandler, setIndex);
}

mtd::Texture::~Texture()
{
	device.freeMemory(imageMemory);
	device.destroyImage(image);
	device.destroyImageView(imageView);
	device.destroySampler(sampler);
}

// Loads texture from file
void mtd::Texture::loadFromFile
(
	const Device& mtdDevice,
	const CommandHandler& commandHandler,
	const char* fileName
)
{
	std::string filePath{MTD_RESOURCES_PATH};
	filePath += fileName;
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if(!pixels)
		LOG_ERROR("Failed to load texture \"%s\".", filePath.c_str());

	Image::CreateImageBundle createImageBundle{device};
	createImageBundle.tiling = vk::ImageTiling::eOptimal;
	createImageBundle.usage =
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	createImageBundle.format = vk::Format::eR8G8B8A8Unorm;
	createImageBundle.imageFlags = vk::ImageCreateFlags();
	createImageBundle.dimensions =
		FrameDimensions{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

	Image::createImage(createImageBundle, image);
	Image::createImageMemory
	(
		mtdDevice, image, vk::MemoryPropertyFlagBits::eDeviceLocal, imageMemory
	);

	loadToGpu(mtdDevice, commandHandler);

	free(pixels);

	Image::createImageView
	(
		device,
		image,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		imageView
	);
	createSampler();
}

// Sends the texture data to the GPU
void mtd::Texture::loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler) const
{
	Memory::Buffer stagingBuffer;
	vk::DeviceSize imageSize = width * height * 4;
	Memory::createBuffer
	(
		mtdDevice,
		stagingBuffer,
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
	);
	Memory::copyMemory(device, stagingBuffer.bufferMemory, imageSize, pixels);
	Image::transitionImageLayout
	(
		image, commandHandler, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal
	);
	Image::copyBufferToImage
	(
		image,
		commandHandler,
		stagingBuffer.buffer,
		FrameDimensions{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}
	);
	Image::transitionImageLayout
	(
		image,
		commandHandler,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);

	device.freeMemory(stagingBuffer.bufferMemory);
	device.destroyBuffer(stagingBuffer.buffer);
}

// Creates sampler to define how the texture should be rendered
void mtd::Texture::createSampler()
{
	vk::SamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.flags = vk::SamplerCreateFlags();
	samplerCreateInfo.magFilter = vk::Filter::eLinear;
	samplerCreateInfo.minFilter = vk::Filter::eNearest;
	samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = vk::False;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.compareEnable = vk::False;
	samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerCreateInfo.unnormalizedCoordinates = vk::False;

	vk::Result result = device.createSampler(&samplerCreateInfo, nullptr, &sampler);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create texture sampler. Vulkan result: %d", result);
}

// Configures the texture descriptor set
void mtd::Texture::createDescriptorResource
(
	DescriptorSetHandler& descriptorSetHandler, uint32_t setIndex
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = imageView;
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(setIndex, 0, descriptorImageInfo);
	descriptorSetHandler.writeDescriptorSet(setIndex);
}
