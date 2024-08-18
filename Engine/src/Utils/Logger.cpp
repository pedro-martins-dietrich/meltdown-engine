#include "Logger.hpp"

#include <cstdarg>
#include <iostream>

#define BUFFER_SIZE 1024

void mtd::Logger::log(Level level, const char* message, ...)
{
	char buffer[BUFFER_SIZE];

	va_list args;
	va_start(args, message);
	if(vsnprintf(buffer, BUFFER_SIZE, message, args) < 0)
	{
		std::cerr << "[LOG ERROR] Failed to parse log message: \"" << message << '\"' << std::endl;
		return;
	}
	va_end(args);

	switch(level)
	{
		case Level::VERBOSE:
			std::cout << "[VERBOSE] " << buffer << '\n';
			break;
		case Level::INFO:
			std::cout << "[INFO] " << buffer << '\n';
			break;
		case Level::WARNING:
			std::cerr << "[WARNING] " << buffer << '\n';
			break;
		case Level::ERROR:
			std::cerr << "[ERROR] " << buffer << std::endl;
			break;
		default:
			std::cout << "[LOG] " << buffer << '\n';
	}
}
