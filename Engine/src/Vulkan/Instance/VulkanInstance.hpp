#pragma once

#include <memory>

#include <meltdown/structs.hpp>

#include "../../Window/Window.hpp"

namespace mtd
{
	// Vulkan instance handler
	class VulkanInstance
	{
		public:
			VulkanInstance(const EngineInfo& info, const Window& window);
			~VulkanInstance();

			VulkanInstance(const VulkanInstance&) = delete;
			VulkanInstance& operator=(const VulkanInstance&) = delete;

			// Getters
			const vk::Instance& getInstance() const { return instance; }
			const vk::SurfaceKHR& getSurface() const { return surface; }

		private:
			// Vulkan instance
			vk::Instance instance;

			// Dispatch loader dynamic with only instance functions
			std::unique_ptr<vk::detail::DispatchLoaderDynamic> dispatchLoader;
			// Vulkan debug messenger (it depends on the validation layer)
			vk::DebugUtilsMessengerEXT debugMessenger;

			// Vulkan surface for GLFW window
			vk::SurfaceKHR surface;

			// Verifies if Vulkan version is compatible with the engine
			uint32_t checkVulkanVersion() const;
			// Checks if required extensions and layers are available
			bool supports
			(
				const std::vector<const char*>& requiredExtensions,
				const std::vector<const char*>& requiredLayers
			);
	};
}
