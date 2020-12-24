#ifndef $_MASONC_MESSAGE_HPP_$
#define $_MASONC_MESSAGE_HPP_$

#include <common.hpp>
#include <location.hpp>
#include <build.hpp>

#include <string>
#include <vector>

namespace masonc
{
    using masonc::lexer::token_location;

    struct message
    {
        token_location location;

        std::string msg;
        build_stage stage;
    };

    struct message_list
    {
        std::vector<message> messages;
        std::vector<message> warnings;
        std::vector<message> errors;

        void print_messages();
        void print_warnings();
        void print_errors();
        void print();

        void report_message(const std::string& msg, build_stage stage = build_stage::UNSET,
            const token_location& location = token_location{ 0, 0, 0 });

        void report_warning(const std::string& msg, build_stage stage = build_stage::UNSET,
            const token_location& location = token_location{ 0, 0, 0 });

        void report_error(const std::string& msg, build_stage stage = build_stage::UNSET,
            const token_location& location = token_location{ 0, 0, 0 });
    };
}

#endif