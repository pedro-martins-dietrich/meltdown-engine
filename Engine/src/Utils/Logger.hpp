#pragma once

// Helper functions to log data according to build
namespace mtd::Logger
{
	enum class Level
	{
		VERBOSE,
		INFO,
		WARNING,
		ERROR
	};

	void log(Level level, const char* message, ...);
}

// Prints detailed data for Debug mode
#if MTD_DEBUG >= 2
	#define LOG_VERBOSE(message, ...) Logger::log(mtd::Logger::Level::VERBOSE, message, ##__VA_ARGS__)
#else
	#define LOG_VERBOSE(message, ...)
#endif

// Prints data for Debug and RelWithDebInfo modes
#if MTD_DEBUG >= 1
	#define LOG_INFO(message, ...) Logger::log(mtd::Logger::Level::INFO, message, ##__VA_ARGS__)
#else
	#define LOG_INFO(message, ...)
#endif

// Prints warning messages
#ifdef MTD_DEBUG
	#define LOG_WARNING(message, ...) Logger::log(mtd::Logger::Level::WARNING, message, ##__VA_ARGS__)
#else
	#define LOG_WARNING(message, ...)
#endif

// Prints error messages
#define LOG_ERROR(message, ...) Logger::log(mtd::Logger::Level::ERROR, message, ##__VA_ARGS__)
