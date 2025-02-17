#include <pch.hpp>
#include "PhysicalDevice.hpp"

#include "../../Utils/Logger.hpp"

mtd::PhysicalDevice::PhysicalDevice(const vk::Instance& vulkanInstance) : physicalDevice{nullptr}
{
	std::vector<vk::PhysicalDevice> availableDevices = vulkanInstance.enumeratePhysicalDevices();
	LOG_VERBOSE("There are %d device(s) available.", availableDevices.size());

	std::array<vk::PhysicalDeviceType, 5> physicalDeviceTypePreferenceList
	{
		vk::PhysicalDeviceType::eDiscreteGpu,
		vk::PhysicalDeviceType::eIntegratedGpu,
		vk::PhysicalDeviceType::eVirtualGpu,
		vk::PhysicalDeviceType::eCpu,
		vk::PhysicalDeviceType::eOther
	};

	for(vk::PhysicalDeviceType type: physicalDeviceTypePreferenceList)
	{
		selectPhysicalDevice(availableDevices, type);
		if(physicalDevice) break;
	}

	if(!physicalDevice)
	{
		LOG_ERROR("Failed to select a physical device.");
		return;
	}

	LOG_INFO("Selected physical device: %s.\n", physicalDevice.getProperties().deviceName);
}

// Verifies if the hardware supports ray tracing
bool mtd::PhysicalDevice::isRayTracingCompatible() const
{
	constexpr std::array<const char*, 3> rayTracingExtensions =
	{
		vk::KHRRayTracingPipelineExtensionName,
		vk::KHRAccelerationStructureExtensionName,
		vk::KHRDeferredHostOperationsExtensionName
	};
	const std::vector<vk::ExtensionProperties> availableExtensions =
		physicalDevice.enumerateDeviceExtensionProperties();

	for(const char* extension: rayTracingExtensions)
	{
		bool found = false;
		for(const vk::ExtensionProperties& availableExtension: availableExtensions)
		{
			if(!strcmp(availableExtension.extensionName.data(), extension))
			{
				found = true;
				LOG_VERBOSE("Vulkan extension \"%s\" is supported.", extension);
				break;
			}
		}
		if(!found)
		{
			LOG_VERBOSE("Vulkan extension \"%s\" is not supported by the hardware.", extension);
			return false;
		}
	}
	return true;
}

// Selects a physical with the specified type
void mtd::PhysicalDevice::selectPhysicalDevice
(
	const std::vector<vk::PhysicalDevice>& availableDevices, vk::PhysicalDeviceType type
)
{
	for(const vk::PhysicalDevice& availableDevice: availableDevices)
	{
		if(availableDevice.getProperties().deviceType == type && isDeviceSuitable(availableDevice))
		{
			physicalDevice = availableDevice;
			break;
		}
	}
}

// Checks if the available device is appropriate for usage
bool mtd::PhysicalDevice::isDeviceSuitable(const vk::PhysicalDevice& availableDevice)
{
	const std::vector<vk::ExtensionProperties> availableExtensions =
		availableDevice.enumerateDeviceExtensionProperties();

	for(const vk::ExtensionProperties& availableExtension: availableExtensions)
	{
		if(!strcmp(availableExtension.extensionName.data(), vk::KHRSwapchainExtensionName))
			return true;
	}

	return false;
}
