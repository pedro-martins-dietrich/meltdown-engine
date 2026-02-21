#pragma once

#ifndef MTD_SHARED
	#define MELTDOWN_API
#elif defined(_WIN32)
	#ifdef MTD_EXPORTS
		#define MELTDOWN_API __declspec(dllexport)
	#else
		#define MELTDOWN_API __declspec(dllimport)
	#endif
#elif defined(__linux__) || defined(__APPLE__)
	#ifdef MTD_EXPORTS
		#define MELTDOWN_API __attribute__((visibility("default")))
	#else
		#define MELTDOWN_API
	#endif
#else
	#define MELTDOWN_API
#endif
