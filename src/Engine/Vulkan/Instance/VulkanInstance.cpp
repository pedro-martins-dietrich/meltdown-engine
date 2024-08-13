#include "VulkanInstance.hpp"

#include "DebugMessenger.hpp"
#include "../../Utils/Logger.hpp"

mtd::VulkanInstance::VulkanInstance(const char* appName, uint32_t appVersion, const Window& window)
{
	uint32_t version = checkVulkanVersion();

	vk::ApplicationInfo appInfo{};
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = appVersion;
	appInfo.pEngineName = "Meltdown Engine";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.apiVersion = version;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> requiredExtensions{glfwExtensions, glfwExtensions + glfwExtensionCount};
	std::vector<const char*> requiredLayers;

	#ifdef MTD_DEBUG
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
	#endif

	if(!supports(requiredExtensions, requiredLayers))
		throw std::runtime_error("Extensions or layers not supported.\n");

	vk::InstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.flags = vk::InstanceCreateFlags();
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if(vk::createInstance(&instanceCreateInfo, nullptr, &instance) != vk::Result::eSuccess)
		throw std::runtime_error("Failed to create Vulkan instance.\n");
	LOG_INFO("Vulkan instance created.");

	dispatchLoader = std::make_unique<vk::DispatchLoaderDynamic>(instance, vkGetInstanceProcAddr);
	#ifdef MTD_DEBUG
		DebugMessenger::createDebugMessenger(instance, dispatchLoader.get(), &debugMessenger);
	#endif

	surface = window.createSurface(instance);
}

mtd::VulkanInstance::~VulkanInstance()
{
	instance.destroySurfaceKHR(surface);

	#ifdef MTD_DEBUG
		instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, *dispatchLoader);
	#endif

	instance.destroy();
}

// Verifies if Vulkan version is compatible with the engine
uint32_t mtd::VulkanInstance::checkVulkanVersion() const
{
	uint32_t version = 0;
	vk::Result result = vk::enumerateInstanceVersion(&version);
	uint32_t requestedVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

	LOG_VERBOSE
	(
		"Vulkan version: %d.%d.%d\t\tvariant: %d",
		VK_API_VERSION_MAJOR(version),
		VK_API_VERSION_MINOR(version),
		VK_API_VERSION_PATCH(version),
		VK_API_VERSION_VARIANT(version)
	);
	LOG_VERBOSE
	(
		"Requested Vulkan version: %d.%d.%d\tvariant: %d",
		VK_API_VERSION_MAJOR(requestedVersion),
		VK_API_VERSION_MINOR(requestedVersion),
		VK_API_VERSION_PATCH(requestedVersion),
		VK_API_VERSION_VARIANT(requestedVersion)
	);

	if
	(
		VK_API_VERSION_VARIANT(version) != VK_API_VERSION_VARIANT(requestedVersion) ||
		VK_API_VERSION_MAJOR(version) < VK_API_VERSION_MAJOR(requestedVersion) ||
		VK_API_VERSION_MINOR(version) < VK_API_VERSION_MINOR(requestedVersion) ||
		VK_API_VERSION_PATCH(version) < VK_API_VERSION_PATCH(requestedVersion)
	)
	{
		throw std::runtime_error("Vulkan version detected is not compatible.\n");
	}

	return version;
}

// Checks if required extensions and layers are available
bool mtd::VulkanInstance::supports
(
	std::vector<const char*> requiredExtensions,
	std::vector<const char*> requiredLayers
)
{
	std::vector<vk::ExtensionProperties> supportedExtensions =
		vk::enumerateInstanceExtensionProperties();
	std::vector<vk::LayerProperties> supportedLayers =
		vk::enumerateInstanceLayerProperties();

	bool found;
	for(const char* requiredExtension: requiredExtensions)
	{
		found = false;
		for(vk::ExtensionProperties& supportedExtension: supportedExtensions)
		{
			if(strcmp(requiredExtension, supportedExtension.extensionName) == 0)
			{
				LOG_VERBOSE("Extension \"%s\" is supported.", requiredExtension);
				found = true;
				break;
			}
		}
		if(!found)
		{
			LOG_WARNING("Extension \"%s\" is not supported.", requiredExtension);
			return false;
		}
	}
	for(const char* requiredLayer: requiredLayers)
	{
		found = false;
		for(vk::LayerProperties& supportedLayer: supportedLayers)
		{
			if(strcmp(requiredLayer, supportedLayer.layerName) == 0)
			{
				LOG_VERBOSE("Layer \"%s\" is supported.", requiredLayer);
				found = true;
				break;
			}
		}
		if(!found)
		{
			LOG_WARNING("Layer \"%s\" is not supported.", requiredLayer);
			return false;
		}
	}

	return true;
}
