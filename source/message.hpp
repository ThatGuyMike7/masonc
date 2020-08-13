#ifndef _MASON_MESSAGE_HPP_$
#define _MASON_MESSAGE_HPP_$

#include <string>
#include <vector>

#include <common.hpp>
#include <build.hpp>

namespace masonc
{
	struct message
	{
		u64 line_number;
        u64 start_column;
        u64 end_column;

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
            u64 line_number = 0, u64 start_column = 0, u64 end_column = 0);

        void report_warning(const std::string& msg, build_stage stage = build_stage::UNSET,
            u64 line_number = 0, u64 start_column = 0, u64 end_column = 0);

        void report_error(const std::string& msg, build_stage stage = build_stage::UNSET,
            u64 line_number = 0, u64 start_column = 0, u64 end_column = 0);
    };
}

#endif