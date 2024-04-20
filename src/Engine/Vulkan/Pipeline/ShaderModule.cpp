#include "ShaderModule.hpp"

#include "../../Utils/FileHandler.hpp"
#include "../../Utils/Logger.hpp"

#ifndef MTD_SPIRV_SHADERS_PATH
	#define MTD_SPIRV_SHADERS_PATH "./resources/shaders/"
#endif

mtd::ShaderModule::ShaderModule(const char* shaderFile, const vk::Device& device)
	: device{device}
{
	std::string shaderPath{MTD_SPIRV_SHADERS_PATH};
	shaderPath.append(shaderFile);

	std::vector<char> shaderSourceCode;
	FileHandler::readFile(shaderPath.c_str(), shaderSourceCode);

	vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.flags = vk::ShaderModuleCreateFlags();
	shaderModuleCreateInfo.codeSize = shaderSourceCode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderSourceCode.data());

	vk::Result result = device.createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule);
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create shader module for \"%s\".", shaderPath.c_str());
}

mtd::ShaderModule::~ShaderModule()
{
	device.destroyShaderModule(shaderModule);
}
