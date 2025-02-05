#pragma once

// The debug messenger is only compiled for debug builds
#ifdef MTD_DEBUG
	#include <vulkan/vulkan.hpp>

	// Handles Vulkan debug messages
	namespace mtd::DebugMessenger
	{
		// Configures Vulkan debug messenger
		void createDebugMessenger
		(
			const vk::Instance& instance,
			const vk::detail::DispatchLoaderDynamic* pDispatchLoader,
			vk::DebugUtilsMessengerEXT* pDebugMessenger
		);
	}
#endif
