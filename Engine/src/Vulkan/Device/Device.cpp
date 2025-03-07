#include <pch.hpp>
#include "Device.hpp"

#include "../../Utils/Logger.hpp"

mtd::Device::Device(const VulkanInstance& vulkanInstance, bool tryEnableRayTracing)
	: device{nullptr},
	physicalDevice{vulkanInstance.getInstance()},
	queueFamilies{physicalDevice.getPhysicalDevice(), vulkanInstance.getSurface()},
	rayTracingEnabled{tryEnableRayTracing && physicalDevice.isRayTracingCompatible()}
{
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
	configureQueues(deviceQueueCreateInfos);

	std::vector<const char*> enabledLayers;
	#ifdef MTD_DEBUG
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif

	std::vector<const char*> extensions;
	selectExtensions(extensions);

	vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures{};
	bufferDeviceAddressFeatures.bufferDeviceAddress = vk::True;
	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationFeatures{};
	accelerationFeatures.accelerationStructure = vk::True;
	accelerationFeatures.pNext = &bufferDeviceAddressFeatures;
	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures{};
	rayTracingFeatures.rayTracingPipeline = vk::True;
	rayTracingFeatures.pNext = &accelerationFeatures;

	vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
	vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2{};
	physicalDeviceFeatures2.pNext = &rayTracingFeatures;

	physicalDevice.getPhysicalDevice().getFeatures2(&physicalDeviceFeatures2);
	if(!(rayTracingFeatures.rayTracingPipeline && accelerationFeatures.accelerationStructure))
		rayTracingEnabled = false;

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.flags = vk::DeviceCreateFlags();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
	deviceCreateInfo.pEnabledFeatures = rayTracingEnabled ? nullptr : &physicalDeviceFeatures;
	deviceCreateInfo.pNext = rayTracingEnabled ? &physicalDeviceFeatures2 : nullptr;

	vk::Result result = physicalDevice.getPhysicalDevice().createDevice(&deviceCreateInfo, nullptr, &device);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed to create logical device. Vulkan result: %d", result);
		return;
	}
	LOG_INFO("Created logical device.\n");

	dldi = std::make_unique<vk::detail::DispatchLoaderDynamic>
	(
		vulkanInstance.getInstance(), vkGetInstanceProcAddr,
		device, vkGetDeviceProcAddr
	);
}

mtd::Device::~Device()
{
	device.destroy();
}

// Configure the Vulkan queues
void mtd::Device::configureQueues(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos) const
{
	std::vector<uint32_t> uniqueQueueFamilyIndices = {queueFamilies.getGraphicsFamilyIndex()};

	if(queueFamilies.getGraphicsFamilyIndex() != queueFamilies.getPresentFamilyIndex())
		uniqueQueueFamilyIndices.push_back(queueFamilies.getPresentFamilyIndex());

	float queuePriority = 1.0f;
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
}

// Selects the Vulkan extensions to be used
void mtd::Device::selectExtensions(std::vector<const char*>& extensions) const
{
	if(rayTracingEnabled)
	{
		extensions =
		{
			vk::KHRSwapchainExtensionName,
			vk::KHRRayTracingPipelineExtensionName,
			vk::KHRAccelerationStructureExtensionName,
			vk::KHRDeferredHostOperationsExtensionName,
			vk::KHRBufferDeviceAddressExtensionName
		};
	}
	else
	{
		extensions = {vk::KHRSwapchainExtensionName};
	}
}
