#pragma once

#include <stb_image.h>

#include "Image.hpp"
#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Handles textures to be passed to the GPU
	class Texture
	{
		public:
			Texture(const Device& mtdDevice, const char* fileName);
			Texture
			(
				const Device& mtdDevice,
				const char* fileName,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex,
				uint32_t binding
			);
			~Texture() = default;

			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

			Texture(Texture&& other) noexcept;

			// Getter
			vk::DescriptorImageInfo getDescriptorImageInfo() const;

			// Sends the texture data to the GPU
			void loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler);

		private:
			// Texture data
			int width, height, channels;
			stbi_uc* pixels;

			// Vulkan image resources
			Image image;
			// Indicates if the texture has failed to load properly
			bool isPlaceholderTexture;

			// Loads texture from file
			void loadFromFile(const Device& mtdDevice, const char* fileName);
			// Configures the texture descriptor set
			void createDescriptorResource
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t binding
			) const;
	};
}
