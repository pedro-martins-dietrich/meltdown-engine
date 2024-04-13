#include "DebugMessenger.hpp"

// The debug messenger is only compiled for debug builds
#ifdef MTD_DEBUG
	#include "../../Utils/Logger.hpp"

	// Vulkan messenger callback
	VKAPI_ATTR vk::Bool32 VKAPI_CALL mtd::DebugMessenger::debugCallback
	(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::string message = "[VULKAN VALIDATION LAYER] ";
		switch(messageType)
		{
			case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
				message.append("[GENERAL]\n\t");
				break;
			case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
				message.append("[VALIDATION]\n\t");
				break;
			case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
				message.append("[PERFORMANCE]\n\t");
				break;
		}

		message.append(pCallbackData->pMessage).append("\n");

		switch(messageSeverity)
		{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
				LOG_VERBOSE(message.c_str());
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
				LOG_INFO(message.c_str());
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				LOG_WARNING(message.c_str());
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				LOG_ERROR(message.c_str());
				break;
		}

		return vk::False;
	}

	// Confiugures Vulkan debug messenger
	vk::DebugUtilsMessengerEXT mtd::DebugMessenger::createDebugMessenger
	(
		const vk::Instance& instance,
		const vk::DispatchLoaderDynamic& dispatchLoader
	)
	{
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		debugMessengerInfo.flags = vk::DebugUtilsMessengerCreateFlagsEXT();
		debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debugMessengerInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT) debugCallback;

		return instance.createDebugUtilsMessengerEXT(debugMessengerInfo, nullptr, dispatchLoader);
	}
#endif
