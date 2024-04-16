#include "Device.hpp"

#include "../../Utils/Logger.hpp"

mtd::Device::Device(const VulkanInstance& vulkanInstance)
	: device{nullptr},
	physicalDevice{vulkanInstance.getInstance()},
	queueFamilies{physicalDevice.getPhysicalDevice(), vulkanInstance.getSurface()}
{
	std::vector<uint32_t> uniqueQueueFamilyIndices = {queueFamilies.getGraphicsFamilyIndex()};

	if(queueFamilies.getGraphicsFamilyIndex() != queueFamilies.getPresentFamilyIndex())
		uniqueQueueFamilyIndices.push_back(queueFamilies.getPresentFamilyIndex());

	float queuePriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
	for(uint32_t uniqueQueueFamilyIndex: uniqueQueueFamilyIndices)
	{
		deviceQueueCreateInfos.emplace_back
		(
			vk::DeviceQueueCreateFlags(),
			uniqueQueueFamilyIndex,
			1,
			&queuePriority
		);
	}

	std::vector<const char*> enabledLayers;
	#ifdef MTD_DEBUG
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif

	const char* deviceExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.flags = vk::DeviceCreateFlags();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = &deviceExtension;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	vk::Result result = physicalDevice.getPhysicalDevice().createDevice(&deviceCreateInfo, nullptr, &device);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create logical device. Vulkan result: %d", result);
		return;
	}

	LOG_INFO("Created logical device.\n");
}

mtd::Device::~Device()
{
	device.destroy();
}
