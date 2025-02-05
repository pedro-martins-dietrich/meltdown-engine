#include <pch.hpp>
#include "DebugMessenger.hpp"

// The debug messenger is only compiled for debug builds
#ifdef MTD_DEBUG
	#include "../../Utils/Logger.hpp"

	// Vulkan messenger callback
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback
	(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::string message = "\x1b[94m" "[VULKAN VALIDATION LAYER]" "\x1b[0m ";
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
			default:
				message.append("[OTHER]\n\t");
		}

		message.append(pCallbackData->pMessage).append("\n");

		switch(messageSeverity)
		{
			using namespace mtd;
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

	// Configures Vulkan debug messenger
	void mtd::DebugMessenger::createDebugMessenger
	(
		const vk::Instance& instance,
		const vk::detail::DispatchLoaderDynamic* pDispatchLoader,
		vk::DebugUtilsMessengerEXT* pDebugMessenger
	)
	{
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		debugMessengerInfo.flags = vk::DebugUtilsMessengerCreateFlagsEXT();
		debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debugMessengerInfo.pfnUserCallback = (vk::PFN_DebugUtilsMessengerCallbackEXT) debugCallback;

		if(instance.createDebugUtilsMessengerEXT
		(
			&debugMessengerInfo, nullptr, pDebugMessenger, *pDispatchLoader
		) != vk::Result::eSuccess)
		{
			LOG_ERROR("Failed to create Vulkan debug messenger.");
		}
	}
#endif
