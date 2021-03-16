#include <logger.hpp>

#include <common.hpp>

#include <iostream>

namespace masonc
{
    void internal_logger::log_message(const char* message)
    {
        buffer_mutex.lock();
        buffer.push_back(log_element{ message, log_element::MESSAGE });
        buffer_mutex.unlock();
    }

    void internal_logger::log_warning(const char* message)
    {
        buffer_mutex.lock();
        buffer.push_back(log_element{ message, log_element::WARNING });
        buffer_mutex.unlock();
    }

    void internal_logger::log_error(const char* message)
    {
        buffer_mutex.lock();
        buffer.push_back(log_element{ message, log_element::ERROR });
        buffer_mutex.unlock();
    }

    void internal_logger::flush()
    {
        buffer_mutex.lock();

        for (u64 i = 0; i < buffer.size(); i += 1) {
            log_element* current = &buffer[i];

            switch (current->type) {
                case log_element::MESSAGE:
                    std::cout << "internal message: " << current->string << '\n';
                    break;
                case log_element::WARNING:
                    std::cout << "internal warning: " << current->string << '\n';
                    break;
                case log_element::ERROR:
                    std::cout << "internal error: " << current->string << '\n';
                    break;
            }
        }

        std::cout << std::flush;
        buffer_mutex.unlock();
    }
}