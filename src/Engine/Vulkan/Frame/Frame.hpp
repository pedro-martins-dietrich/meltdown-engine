#pragma once

#include <vulkan/vulkan.hpp>

#include "../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Defines the properties of each frame
	class Frame
	{
		public:
			Frame
			(
				const vk::Device& device,
				const FrameDimensions& frameDimensions,
				vk::Image image,
				vk::Format format
			);
			~Frame();

			Frame(const Frame&) = delete;
			Frame& operator=(const Frame&) = delete;

			Frame(Frame&& otherFrame) noexcept;

		private:
			// Image data
			vk::Image image;
			// Image desctiption
			vk::ImageView imageView;

			// Frame dimensions
			FrameDimensions frameDimensions;

			// Vulkan device reference
			const vk::Device& device;

			// Create frame image view
			void createImageView(vk::Format format);
	};
}
