#include <pch.hpp>
#include "ShaderModule.hpp"

#include "../../Utils/FileHandler.hpp"
#include "../../Utils/Logger.hpp"

mtd::ShaderModule::ShaderModule(const vk::Device& device, vk::ShaderStageFlagBits shaderStage, const char* shaderFile)
	: device{device}, shaderStage{shaderStage}
{
	std::string shaderPath{MTD_RESOURCES_PATH};
	shaderPath.append("shaders/");
	shaderPath.append(shaderFile);

	std::vector<char> shaderSourceCode;
	if(!FileHandler::readFile(shaderPath.c_str(), shaderSourceCode))
		return;

	vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.flags = vk::ShaderModuleCreateFlags();
	shaderModuleCreateInfo.codeSize = shaderSourceCode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderSourceCode.data());

	vk::Result result = device.createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule);
	if(result != vk::Result::eSuccess)
	{
		LOG_ERROR
		(
			"Failed to create shader module for \"%s\". Vulkan result: %d",
			shaderPath.c_str(), result
		);
		return;
	}
	LOG_VERBOSE("Loaded shader module: \"%s\"", shaderPath.c_str());
}

mtd::ShaderModule::~ShaderModule()
{
	device.destroyShaderModule(shaderModule);
}

mtd::ShaderModule::ShaderModule(ShaderModule&& other) noexcept
	: device{other.device},
	shaderModule{std::move(other.shaderModule)},
	shaderStage{other.shaderStage}
{
	shaderModule = nullptr;
}

// Creates the Vulkan pipeline shader stage create info
vk::PipelineShaderStageCreateInfo mtd::ShaderModule::generatePipelineShaderCreateInfo() const
{
	return {vk::PipelineShaderStageCreateFlags(), shaderStage, shaderModule, "main", nullptr};
}
