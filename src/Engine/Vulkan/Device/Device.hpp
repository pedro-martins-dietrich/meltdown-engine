#pragma once

#include "../Instance/VulkanInstance.hpp"
#include "PhysicalDevice.hpp"
#include "QueueFamilies.hpp"

namespace mtd
{
	// Handles the Vulkan logical device (abstraction of the physical device)
	class Device
	{
		public:
			Device(const VulkanInstance& vulkanInstance);
			~Device();

			Device(const Device&) = delete;
			Device& operator=(const Device&) = delete;

			// Device getters
			const vk::Device& getDevice() const { return device; }
			const vk::PhysicalDevice& getPhysicalDevice() const
				{ return physicalDevice.getPhysicalDevice(); }

			// Queue getters
			const QueueFamilies& getQueueFamilies() const { return queueFamilies; }
			const vk::Queue getGraphicsQueue() const
			{
				return device.getQueue(queueFamilies.getGraphicsFamilyIndex(), 0);
			}
			const vk::Queue getPresentQueue() const
			{
				return device.getQueue(queueFamilies.getPresentFamilyIndex(), 0);
			}

		private:
			// Vulkan logical device
			vk::Device device;
			// Physical device handler
			PhysicalDevice physicalDevice;
			// Queue family handler
			QueueFamilies queueFamilies;
	};
}
