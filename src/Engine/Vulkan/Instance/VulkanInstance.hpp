#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Vulkan instance handler
	class VulkanInstance
	{
		public:
			VulkanInstance(const char* appName, uint32_t appVersion);
			~VulkanInstance();

			VulkanInstance(const VulkanInstance&) = delete;
			VulkanInstance& operator=(const VulkanInstance&) = delete;

			// Getter
			const vk::Instance& getInstance() const { return instance; }

		private:
			// Vulkan instance
			vk::Instance instance;
			// Dispatch loader dynamic instance
			vk::DispatchLoaderDynamic dispatchLoader;

			// Verifies if Vulkan version is compatible with the engine
			uint32_t checkVulkanVersion();
			// Checks if required extensions and layers are available
			bool supports
			(
				std::vector<const char*> requiredExtensions,
				std::vector<const char*> requiredLayers
			);

			// Creates an instance of the dispatch loader
			void createDispatchLoader();
	};
}
