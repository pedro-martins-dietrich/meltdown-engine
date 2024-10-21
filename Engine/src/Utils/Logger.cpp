#include <pch.hpp>
#include "Logger.hpp"

#include <cstdarg>

static constexpr int bufferSize = 2048;

static constexpr const char* defaultColor = "\x1b[0m";
static constexpr const char* redColor = "\x1b[31m";
static constexpr const char* yellowColor = "\x1b[33m";
static constexpr const char* brightBlackColor = "\x1b[90m";
static constexpr const char* brightRedColor = "\x1b[91m";
static constexpr const char* brightGreenColor = "\x1b[92m";
static constexpr const char* brightCyanColor = "\x1b[96m";

void mtd::Logger::log(Level level, const char* message, ...)
{
	char buffer[bufferSize];

	va_list args;
	va_start(args, message);
	if(vsnprintf(buffer, bufferSize, message, args) < 0)
	{
		std::cerr << redColor << "[LOG ERROR] " << defaultColor <<
			"Failed to parse log message: \"" << message << '\"' << std::endl;
		return;
	}
	va_end(args);

	switch(level)
	{
		case Level::Verbose:
			std::cout << brightBlackColor << "[VERBOSE] " << defaultColor << buffer << '\n';
			break;
		case Level::Info:
			std::cout << brightGreenColor << "[INFO] " << defaultColor << buffer << '\n';
			break;
		case Level::Warning:
			std::cerr << yellowColor << "[WARNING] " << defaultColor << buffer << '\n';
			break;
		case Level::Error:
			std::cerr << brightRedColor << "[ERROR] " << defaultColor << buffer << std::endl;
			break;
		default:
			std::cout << brightCyanColor << "[LOG] " << defaultColor << buffer << '\n';
	}
}
