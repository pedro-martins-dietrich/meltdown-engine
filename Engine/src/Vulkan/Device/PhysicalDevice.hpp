#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Handles the Vulkan object representing the GPU
	class PhysicalDevice
	{
		public:
			PhysicalDevice(const vk::Instance& vulkanInstance);
			~PhysicalDevice() = default;

			PhysicalDevice(const PhysicalDevice&) = delete;
			PhysicalDevice& operator=(const PhysicalDevice&) = delete;

			// Getter
			const vk::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }

			// Verifies if the hardware supports ray tracing
			bool isRayTracingCompatible() const;

			// Acquires the physical device ray tracing properties
			const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& fetchRayTracingProperties() const
				{ return rayTracingProperties; }

		private:
			// Vulkan representation of the GPU (physical device) to be used
			vk::PhysicalDevice physicalDevice;

			// Properties of the physical device
			vk::PhysicalDeviceProperties2 properties;
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties;

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
