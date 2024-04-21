#pragma once

#include <vulkan/vulkan.hpp>

// Handles synchronazation objects
namespace mtd::Synchronization
{
	// Creates a Vulkan fence
	void createFence(const vk::Device& device, vk::Fence& fence);

	// Creates a Vulkan semaphore
	void createSemaphore(const vk::Device& device, vk::Semaphore& semaphore);
}