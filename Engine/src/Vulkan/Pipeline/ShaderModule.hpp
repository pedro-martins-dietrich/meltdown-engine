#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Handles the SPIR-V compiled shader code
	class ShaderModule
	{
		public:
			ShaderModule(const vk::Device& device, vk::ShaderStageFlagBits shaderStage, const char* shaderFile);
			~ShaderModule();

			ShaderModule(const ShaderModule&) = delete;
			ShaderModule& operator=(const ShaderModule&) = delete;

			ShaderModule(ShaderModule&& other) noexcept;

			// Creates the Vulkan pipeline shader stage create info
			vk::PipelineShaderStageCreateInfo generatePipelineShaderCreateInfo() const;

		private:
			// Representation of the compiled shader code
			vk::ShaderModule shaderModule;

			// Indicator of the shader type
			vk::ShaderStageFlagBits shaderStage;

			// Vulkan device reference
			const vk::Device& device;
	};
}
