#pragma once

#include <optional>

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Finds and sets desired queue family indices
	class QueueFamilies
	{
		public:
			QueueFamilies(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface);
			~QueueFamilies() {}

			QueueFamilies(const QueueFamilies&) = delete;
			QueueFamilies& operator=(const QueueFamilies&) = delete;

			// Getters
			uint32_t getGraphicsFamilyIndex() const { return graphicsFamilyIndex.value(); }
			uint32_t getPresentFamilyIndex() const { return presentFamilyIndex.value(); }

		private:
			// Stores the queue family index that supports graphics computation
			std::optional<uint32_t> graphicsFamilyIndex;
			// Stores the queue family index that supports surface presentation
			std::optional<uint32_t> presentFamilyIndex;

			// Checks if all queue famuly indices have been set
			inline bool isComplete() const
			{
				return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
			}
	};
}
