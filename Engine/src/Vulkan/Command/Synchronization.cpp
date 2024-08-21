#include "Synchronization.hpp"

#include "../../Utils/Logger.hpp"

// Creates a Vulkan fence
void mtd::Synchronization::createFence(const vk::Device& device, vk::Fence& fence)
{
	vk::FenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

	vk::Result result = device.createFence(&fenceCreateInfo, nullptr, &fence);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create fence. Vulkan result: %d", result);
}

// Creates a Vulkan semaphore
void mtd::Synchronization::createSemaphore(const vk::Device& device, vk::Semaphore& semaphore)
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();

	vk::Result result = device.createSemaphore(&semaphoreCreateInfo, nullptr, &semaphore);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create semaphore. Vulkan result: %d", result);
}
