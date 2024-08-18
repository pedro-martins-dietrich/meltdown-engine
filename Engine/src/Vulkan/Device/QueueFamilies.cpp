#include "QueueFamilies.hpp"

#include "../../Utils/Logger.hpp"

mtd::QueueFamilies::QueueFamilies
(
	const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface
)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
		physicalDevice.getQueueFamilyProperties();
	LOG_VERBOSE("System can support %d queue families.", queueFamilyProperties.size());

	for(uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsFamilyIndex = i;
			queueFamilyIndices.push_back(i);
			LOG_VERBOSE("Queue family %d is suitable for graphics.", i);
		}

		if(physicalDevice.getSurfaceSupportKHR(i, surface))
		{
			presentFamilyIndex = i;
			if(graphicsFamilyIndex != i)
				queueFamilyIndices.push_back(i);
			LOG_VERBOSE("Queue family %d is suitable for presenting.", i);
		}

		if(isComplete()) break;
	}

	if(!isComplete())
	{
		LOG_ERROR("The available queue families does not support all required features.");
		return;
	}
}

// Checks if all queue famuly indices have been set
bool mtd::QueueFamilies::isComplete() const
{
	return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
}

// Adds queue family index to vector if unique
void mtd::QueueFamilies::addIndexToVector(uint32_t index)
{
	for(uint32_t i: queueFamilyIndices)
		if(i == index) return;

	queueFamilyIndices.push_back(index);
}
