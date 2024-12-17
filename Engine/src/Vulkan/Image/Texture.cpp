#include <pch.hpp>
#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Image.hpp"
#include "../../Utils/Logger.hpp"

mtd::Texture::Texture
(
	const Device& mtdDevice,
	const char* fileName,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t swappableSetIndex,
	uint32_t binding
) : device{mtdDevice.getDevice()}, width{0}, height{0}, channels{0}
{
	loadFromFile(mtdDevice, commandHandler, fileName);
	createDescriptorResource(descriptorSetHandler, swappableSetIndex, binding);
}

mtd::Texture::~Texture()
{
	device.freeMemory(imageMemory);
	device.destroyImage(image);
	device.destroyImageView(imageView);
	device.destroySampler(sampler);
}

mtd::Texture::Texture(Texture&& other) noexcept
	: device{other.device},
	width{other.width}, height{other.height}, channels{other.channels},
	pixels{other.pixels},
	image{std::move(other.image)}, imageMemory{std::move(other.imageMemory)},
	imageView{std::move(other.imageView)}, sampler{std::move(other.sampler)}
{
	other.pixels = nullptr;
	other.image = nullptr;
	other.imageMemory = nullptr;
	other.imageView = nullptr;
	other.sampler = nullptr;
}

// Loads texture from file
void mtd::Texture::loadFromFile(const Device& mtdDevice, const CommandHandler& commandHandler, const char* fileName)
{
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(fileName, &width, &height, &channels, STBI_rgb_alpha);
	if(!pixels)
		LOG_ERROR("Failed to load texture \"%s\".", fileName);

	Image::CreateImageBundle createImageBundle{device};
	createImageBundle.tiling = vk::ImageTiling::eOptimal;
	createImageBundle.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	createImageBundle.format = vk::Format::eR8G8B8A8Unorm;
	createImageBundle.imageFlags = vk::ImageCreateFlags();
	createImageBundle.dimensions = FrameDimensions{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

	Image::createImage(createImageBundle, image);
	Image::createImageMemory(mtdDevice, image, vk::MemoryPropertyFlagBits::eDeviceLocal, imageMemory);

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
	const vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(width * height * 4);
	GpuBuffer stagingBuffer
	{
		mtdDevice,
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
	};
	stagingBuffer.copyMemoryToBuffer(imageSize, pixels);

	Image::transitionImageLayout
	(
		image, commandHandler, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal
	);
	Image::copyBufferToImage
	(
		image,
		commandHandler,
		stagingBuffer.getBuffer(),
		FrameDimensions{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}
	);
	Image::transitionImageLayout
	(
		image,
		commandHandler,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);
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
	DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t binding
) const
{
	vk::DescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = imageView;
	descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	descriptorSetHandler.createImageDescriptorResources(swappableSetIndex, binding, descriptorImageInfo);
	descriptorSetHandler.writeDescriptorSet(swappableSetIndex);
}
