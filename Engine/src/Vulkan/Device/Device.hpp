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
			Device(const VulkanInstance& vulkanInstance, bool tryEnableRayTracing);
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
				{ return device.getQueue(queueFamilies.getGraphicsFamilyIndex(), 0); }
			const vk::Queue getPresentQueue() const
				{ return device.getQueue(queueFamilies.getPresentFamilyIndex(), 0); }

			// Checks if hardware ray tracing is enabled
			bool isRayTracingEnabled() const { return rayTracingEnabled; }

		private:
			// Vulkan logical device
			vk::Device device;
			// Physical device handler
			PhysicalDevice physicalDevice;
			// Queue family handler
			QueueFamilies queueFamilies;

			// Ray tracing hardware support status
			bool rayTracingEnabled;

			// Configures the Vulkan queues
			void configureQueues(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos) const;

			// Selects the Vulkan extensions to be used
			void selectExtensions(std::vector<const char*>& extensions) const;
	};
}
