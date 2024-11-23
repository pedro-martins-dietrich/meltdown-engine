#pragma once

#include <string>

#include <meltdown/enums.hpp>

namespace mtd
{
	/*
	* @brief Information for creating a Meltdown Engine instance.
	* Contains info about the application name and version.
	* Defaults to "Meltdown Application", version 1.0.0.
	* 
	* @param appName Name of the application. Will be shown in the window titlebar.
	* @param appVersionMajor Major version of the application.
	* @param appVersionMinor Minor version of the application.
	* @param appVersionPatch Patch version of the application.
	*/
	struct EngineInfo
	{
		const char* appName = "Meltdown Application";
		unsigned int appVersionMajor = 1;
		unsigned int appVersionMinor = 0;
		unsigned int appVersionPatch = 0;
	};

	/*
	* @brief Parameters used to create a pipeline with custom shaders.
	*
	* @param pipelineName Exhibition name for the pipeline.
	* @param vertexShaderPath File path to the vertex shader from the shaders folder.
	* @param fragmentShaderPath File path to the fragment shader from the shaders folder.
	* @param associatedMeshType Mesh type the pipeline will render.
	* @param primitiveTopology Assembly method for the primitive mesh topology.
	* @param faceCulling Orientation for the pipeline face culling.
	* @param useTransparency Enables or disables alpha blending for the pipeline.
	*/
	struct PipelineInfo
	{
		std::string pipelineName;
		std::string vertexShaderPath;
		std::string fragmentShaderPath;
		MeshType associatedMeshType;
		ShaderPrimitiveTopology primitiveTopology = ShaderPrimitiveTopology::TriangleList;
		ShaderFaceCulling faceCulling = ShaderFaceCulling::None;
		bool useTransparency = false;
	};
}
