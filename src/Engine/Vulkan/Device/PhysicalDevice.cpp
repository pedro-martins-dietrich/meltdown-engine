#include "PhysicalDevice.hpp"

#include "../../Utils/Logger.hpp"

mtd::PhysicalDevice::PhysicalDevice(const vk::Instance& vulkanInstance) : physicalDevice{nullptr}
{
	std::vector<vk::PhysicalDevice> availableDevices = vulkanInstance.enumeratePhysicalDevices();
	LOG_VERBOSE("There are %d devices available.", availableDevices.size());

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
		if(!strcmp(availableExtension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			return true;
	}

	return false;
}
