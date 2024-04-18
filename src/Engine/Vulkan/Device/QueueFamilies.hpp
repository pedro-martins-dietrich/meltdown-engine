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
			const std::vector<uint32_t>& getUniqueIndices() const { return queueFamilyIndices; }

		private:
			// Stores the queue family index that supports graphics computation
			std::optional<uint32_t> graphicsFamilyIndex;
			// Stores the queue family index that supports surface presentation
			std::optional<uint32_t> presentFamilyIndex;

			// Unique queue family indices
			std::vector<uint32_t> queueFamilyIndices;

			// Checks if all queue family indices have been set
			inline bool isComplete() const;
			// Adds queue family index to vector if unique
			void addIndexToVector(uint32_t index);
	};
}
