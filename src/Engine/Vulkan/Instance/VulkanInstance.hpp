#pragma once

#include <vulkan/vulkan.hpp>

#include "../../Window/Window.hpp"

namespace mtd
{
	// Vulkan instance handler
	class VulkanInstance
	{
		public:
			VulkanInstance(const char* appName, uint32_t appVersion, const Window& window);
			~VulkanInstance();

			VulkanInstance(const VulkanInstance&) = delete;
			VulkanInstance& operator=(const VulkanInstance&) = delete;

			// Getters
			const vk::Instance& getInstance() const { return instance; }
			const vk::SurfaceKHR& getSurface() const { return surface; }

		private:
			// Vulkan instance
			vk::Instance instance;
			// Dispatch loader dynamic instance
			vk::DispatchLoaderDynamic dispatchLoader;
			// Vulkan surface for GLFW window
			vk::SurfaceKHR surface;

			// Vulkan debug messenger (it depends on the validation layer)
			#ifdef MTD_DEBUG
				vk::DebugUtilsMessengerEXT debugMessenger;
			#endif

			// Verifies if Vulkan version is compatible with the engine
			uint32_t checkVulkanVersion() const;
			// Checks if required extensions and layers are available
			bool supports
			(
				std::vector<const char*> requiredExtensions,
				std::vector<const char*> requiredLayers
			);
	};
}
