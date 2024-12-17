#pragma once

#include <stb_image.h>

#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Handles textures to be passed to the GPU
	class Texture
	{
		public:
			Texture
			(
				const Device& mtdDevice,
				const char* fileName,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex,
				uint32_t binding
			);
			~Texture();

			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

			Texture(Texture&& other) noexcept;

		private:
			// Texture data
			int width, height, channels;
			stbi_uc* pixels;

			// Vulkan image resources
			vk::Image image;
			vk::DeviceMemory imageMemory;
			vk::ImageView imageView;
			vk::Sampler sampler;

			// Vulkan device reference
			const vk::Device& device;

			// Loads texture from file
			void loadFromFile(const Device& mtdDevice, const CommandHandler& commandHandler, const char* fileName);
			// Sends the texture data to the GPU
			void loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler) const;
			// Creates sampler to define how the texture should be rendered
			void createSampler();
			// Configures the texture descriptor set
			void createDescriptorResource
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t binding
			) const;
	};
}
