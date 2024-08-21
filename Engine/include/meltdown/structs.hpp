#pragma once

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
}
