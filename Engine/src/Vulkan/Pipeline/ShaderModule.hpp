#pragma once

#include <vulkan/vulkan.hpp>

namespace mtd
{
	// Handles the SPIR-V compiled shader code
	class ShaderModule
	{
		public:
			ShaderModule(const char* shaderFile, const vk::Device& device);
			~ShaderModule();

			ShaderModule(const ShaderModule&) = delete;
			ShaderModule& operator=(const ShaderModule&) = delete;

			ShaderModule(ShaderModule&& other) noexcept;

			// Getter
			const vk::ShaderModule& getShaderModule() const { return shaderModule; }

		private:
			// Representation of the compiled shader code
			vk::ShaderModule shaderModule;

			// Vulkan device reference
			const vk::Device& device;
	};
}
