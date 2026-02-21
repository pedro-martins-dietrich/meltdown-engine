#pragma once

#include <memory>

#include <meltdown/structs.hpp>

namespace mtd
{
	// Vulkan instance handler
	class VulkanInstance
	{
		public:
			VulkanInstance(const EngineInfo& info);
			~VulkanInstance();

			VulkanInstance(const VulkanInstance&) = delete;
			VulkanInstance& operator=(const VulkanInstance&) = delete;

			// Getters
			const vk::Instance& getInstance() const { return instance; }

		private:
			// Vulkan instance
			vk::Instance instance;

			// Dispatch loader dynamic with only instance functions
			std::unique_ptr<vk::detail::DispatchLoaderDynamic> dispatchLoader;
			// Vulkan debug messenger (it depends on the validation layer)
			vk::DebugUtilsMessengerEXT debugMessenger;

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
