#include "log.hpp"
#include <iostream>

void log_message(const char* message)
{
	std::cout << "internal message: " << message << std::endl;
}

void log_warning(const char* warning_message)
{
	std::cout << "internal warning: " << warning_message << std::endl;
}

void log_error(const char* error_message)
{
	std::cout << "internal error: " << error_message << std::endl;
}