#include "DescriptorSetHandler.hpp"

#include "../../Utils/Logger.hpp"

mtd::DescriptorSetHandler::DescriptorSetHandler
(
	const vk::Device& device, const std::vector<vk::DescriptorSetLayoutBinding>& setLayoutBindings
) : device{device}
{
	createDescriptorSetLayout(setLayoutBindings);
}

mtd::DescriptorSetHandler::~DescriptorSetHandler()
{
	if(descriptorBuffer.bufferMemory)
	{
		device.unmapMemory(descriptorBuffer.bufferMemory);
		device.freeMemory(descriptorBuffer.bufferMemory);
		device.destroyBuffer(descriptorBuffer.buffer);
	}

	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

mtd::DescriptorSetHandler::DescriptorSetHandler(DescriptorSetHandler&& other) noexcept
	: device{other.device},
	descriptorSetLayout{std::move(other.descriptorSetLayout)},
	descriptorSet{std::move(other.descriptorSet)},
	descriptorBuffer{other.descriptorBuffer},
	descriptorBufferInfo{other.descriptorBufferInfo},
	descriptorBufferWriteLocation{other.descriptorBufferWriteLocation},
	writeOp{other.writeOp},
	descriptorTypes{std::move(other.descriptorTypes)}
{
	other.descriptorSetLayout = nullptr;
	other.descriptorSet = nullptr;
	other.descriptorBuffer.buffer = nullptr;
	other.descriptorBuffer.bufferMemory = nullptr;
}

// Creates a descriptor and assings it to a descriptor set
void mtd::DescriptorSetHandler::createDescriptorResources
(
	const Device& mtdDevice, vk::DeviceSize resourceSize, vk::BufferUsageFlags usageFlags
)
{
	Memory::createBuffer
	(
		mtdDevice,
		descriptorBuffer,
		resourceSize,
		usageFlags,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	descriptorBufferWriteLocation =
		device.mapMemory(descriptorBuffer.bufferMemory, 0UL, resourceSize);
	descriptorBufferInfo.buffer = descriptorBuffer.buffer;
	descriptorBufferInfo.offset = 0UL;
	descriptorBufferInfo.range = resourceSize;
}

// Updates the descriptor set data
void mtd::DescriptorSetHandler::writeDescriptorSet()
{
	writeOp.dstSet = descriptorSet;
	writeOp.dstBinding = 0;
	writeOp.dstArrayElement = 0;
	writeOp.descriptorCount = 1;
	writeOp.descriptorType = descriptorTypes[0];
	writeOp.pImageInfo = nullptr;
	writeOp.pBufferInfo = &descriptorBufferInfo;
	writeOp.pTexelBufferView = nullptr;

	device.updateDescriptorSets(1, &writeOp, 0, nullptr);
}

// Creates a descriptor set layout
void mtd::DescriptorSetHandler::createDescriptorSetLayout
(
	const std::vector<vk::DescriptorSetLayoutBinding>& bindings
)
{
	for(const vk::DescriptorSetLayoutBinding& binding: bindings)
	{
		descriptorTypes.push_back(binding.descriptorType);
	}

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlags();
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	vk::Result result =
		device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &descriptorSetLayout);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create descriptor set layout. Vulkan result: %d", result);
}
