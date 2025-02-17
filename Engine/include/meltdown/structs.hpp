#pragma once

#include <string>

#include <meltdown/enums.hpp>
#include <meltdown/math.hpp>

namespace mtd
{
	/*
	* @brief Information for creating a Meltdown Engine instance.
	* Contains info about the application name and version.
	* Defaults to "Meltdown Application", version 1.0.0.
	*/
	struct EngineInfo
	{
		/* @brief Name of the application. Will be shown in the window titlebar. */
		const char* appName = "Meltdown Application";
		/* @brief Major version of the application. */
		uint32_t appVersionMajor = 1;
		/* @brief Minor version of the application. */
		uint32_t appVersionMinor = 0;
		/* @brief Patch version of the application. */
		uint32_t appVersionPatch = 0;
		/* @brief Flag to enable ray tracing if the hardware supports it. */
		bool enableRayTracing = false;
	};

	/*
	* @brief Information about how a descriptor set binding will be used.
	*/
	struct DescriptorInfo
	{
		/* @brief Data type that will be used in the binding. */
		DescriptorType descriptorType;
		/* @brief Shader stage(s) where the descriptor can be accessed. */
		ShaderStage shaderStage;
		/* @brief Size, in bytes, of the descriptor data. If used as an array, the size of the whole array. */
		size_t totalDescriptorSize;
		/* @brief Array size of the descriptor, if accessed as an array. */
		uint32_t descriptorCount = 1;
	};

	/*
	* @brief Parameters used to create a pipeline with custom shaders.
	*/
	struct PipelineInfo
	{
		/* @brief Exhibition name for the pipeline. */
		std::string pipelineName;
		/* @brief File path to the vertex shader from the shaders folder. */
		std::string vertexShaderPath;
		/* @brief File path to the fragment shader from the shaders folder. */
		std::string fragmentShaderPath;
		/* @brief Mesh type the pipeline will render. */
		MeshType associatedMeshType;
		/* @brief Framebuffer index where the pipeline will render to. Use -1 to render directly to the swapchain. */
		int32_t targetFramebufferIndex = -1;
		/* @brief Info about each binding for the user defined descriptor set (set = 2). */
		std::vector<DescriptorInfo> descriptorSetInfo = {};
		/* @brief Assembly method for the primitive mesh topology. */
		ShaderPrimitiveTopology primitiveTopology = ShaderPrimitiveTopology::TriangleList;
		/* @brief Orientation for the pipeline face culling. */
		ShaderFaceCulling faceCulling = ShaderFaceCulling::None;
		/* @brief Enables alpha blending for the pipeline. */
		bool useTransparency = false;
		/* @brief Types of materials attributes stored as floats used in the pipline. */
		std::vector<MaterialFloatDataType> materialFloatDataTypes = {};
		/* @brief Types of materials attributes stored as textures used in the pipline. */
		std::vector<MaterialTextureType> materialTextureTypes = {};
	};

	/*
	* @brief Parameters used to create a custom framebuffer.
	*/
	struct FramebufferInfo
	{
		/* @brief Attachments used in the framebuffer. */
		FramebufferAttachments framebufferAttachments;
		/* @brief Type of filter used for sampling the attachments. */
		TextureSamplingFilterType samplingFilter;
		/* @brief Ratio between the framebuffer and window resolution. Use negative values for a fixed resolution. */
		Vec2 windowResolutionRatio;
		/* @brief Framebuffer horizontal resolution. Ignored when a positive `windowResolutionRatio.x` is set. */
		uint32_t width;
		/* @brief Framebuffer vertical resolution. Ignored when a positive `windowResolutionRatio.y` is set. */
		uint32_t height;
	};

	/*
	* @brief Indices used to locate a specific framebuffer attachment in the current scene.
	*/
	struct AttachmentIdentifier
	{
		/* @brief Framebuffer index in the current scene. */
		uint32_t framebufferIndex;
		/* @brief Attachment index in the selected framebuffer. */
		uint32_t attachmentIndex;
	};

	/*
	* @brief Parameters used to create a render-to-framebuffer pipeline with custom shaders.
	*/
	struct FramebufferPipelineInfo
	{
		/* @brief Exhibition name for the pipeline. */
		std::string pipelineName;
		/* @brief File path to the vertex shader from the shaders folder. */
		std::string vertexShaderPath;
		/* @brief File path to the fragment shader from the shaders folder. */
		std::string fragmentShaderPath;
		/* @brief Framebuffer index where the pipeline will render to. Use -1 to render to the swapchain. */
		int32_t targetFramebufferIndex = -1;
		/* @brief Info about each binding for the user defined descriptor set (set = 2). */
		std::vector<DescriptorInfo> descriptorSetInfo = {};
		/* @brief Attachments used as input for the pipeline. */
		std::vector<AttachmentIdentifier> inputAttachments = {};
		/* @brief Indices of the pipelines that draw to the input framebuffers. */
		std::vector<uint32_t> dependencies = {};
	};
}
