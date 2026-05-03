#pragma once

#include <stb_image.h>

#include "Image.hpp"
#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Handles a texture to be used in the GPU
	class Texture
	{
		public:
			Texture(const Device& mtdDevice, std::string_view fileName);
			Texture(const Device& mtdDevice, std::string_view fileName, const CommandHandler& commandHandler);
			Texture(const Device& mtdDevice, const CommandHandler& commandHandler);
			Texture
			(
				const Device& mtdDevice,
				std::string_view fileName,
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

			// Updates the descriptor image info with the texture information
			void updateDescriptorImageInfo(vk::DescriptorImageInfo& descriptorImageInfo) const
				{ image.defineDescriptorImageInfo(&descriptorImageInfo); }

			// Sends the texture data to the GPU
			void loadToGpu(const Device& mtdDevice, const CommandHandler& commandHandler);

		private:
			// Texture data
			int channels = 0;
			stbi_uc* pixels = nullptr;

			// Vulkan image resources
			Image image;
			// Indicates if the texture has failed to load properly
			bool isPlaceholderTexture = false;

			// Loads texture from file
			void loadFromFile(const Device& mtdDevice, std::string_view fileName);
			// Loads placeholder texture
			stbi_uc* loadPlaceholderTexture(int& width, int& height);
			// Configures the texture descriptor set
			void createDescriptorResource
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t binding
			) const;
	};
}
