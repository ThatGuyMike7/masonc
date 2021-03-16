#ifndef MASONC_LOGGER_HPP
#define MASONC_LOGGER_HPP

#include <mutex>
#include <vector>

namespace masonc
{
    // Provides thread-safe buffering of internal messages, warnings, and errors.
    struct internal_logger
    {
        struct log_element
        {
            const char* string;

            enum
            {
                MESSAGE, WARNING, ERROR
            } type;
        };

        void log_message(const char* message);
        void log_warning(const char* message);
        void log_error(const char* message);

        // Prints all the logged messages, warnings and errors in the order in which they were added.
        void flush();

    private:
        std::mutex buffer_mutex;
        std::vector<log_element> buffer;
    };

    inline internal_logger global_logger;
}

#endif