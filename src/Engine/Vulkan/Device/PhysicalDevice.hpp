#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Handles the representation of the GPU
	class PhysicalDevice
	{
		public:
			PhysicalDevice(const vk::Instance& vulkanInstance);
			~PhysicalDevice() {}

			PhysicalDevice(const PhysicalDevice&) = delete;
			PhysicalDevice& operator=(const PhysicalDevice&) = delete;

			// Getter
			const vk::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }

		private:
			// Vulkan representation of the GPU (physical device) to be used
			vk::PhysicalDevice physicalDevice;

			// Selects a physical with the specified type
			void selectPhysicalDevice
			(
				const std::vector<vk::PhysicalDevice>& availableDevices,
				vk::PhysicalDeviceType type
			);
			// Checks if the available device is appropriate for usage
			bool isDeviceSuitable(const vk::PhysicalDevice& availableDevice);
	};
}
