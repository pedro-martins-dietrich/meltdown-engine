#pragma once

#include <string>

#include <meltdown/enums.hpp>

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
		/* @brief Info about each binding for the user defined descriptor set (set = 2). */
		std::vector<DescriptorInfo> descriptorSetInfo = {};
		/* @brief Assembly method for the primitive mesh topology. */
		ShaderPrimitiveTopology primitiveTopology = ShaderPrimitiveTopology::TriangleList;
		/* @brief Orientation for the pipeline face culling. */
		ShaderFaceCulling faceCulling = ShaderFaceCulling::None;
		/* @brief Enables or disables alpha blending for the pipeline. */
		bool useTransparency = false;
	};
}
