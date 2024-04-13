#pragma once

// The debug messenger is only compiled for debug builds
#ifdef MTD_DEBUG
	#include <vulkan/vulkan.hpp>

	// Handles Vulkan debug messages
	namespace mtd::DebugMessenger
	{
		// Vulkan messenger callback
		VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback
		(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		// Confiugures Vulkan debug messenger
		vk::DebugUtilsMessengerEXT createDebugMessenger
		(
			const vk::Instance& instance,
			const vk::DispatchLoaderDynamic& dispatchLoader
		);
	}
#endif
