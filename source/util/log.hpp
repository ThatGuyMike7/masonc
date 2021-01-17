#ifndef MASONC_LOG_HPP
#define MASONC_LOG_HPP

namespace masonc
{
    void log_message(const char* message);
    void log_warning(const char* warning_message);
    void log_error(const char* error_message);
}

#endif