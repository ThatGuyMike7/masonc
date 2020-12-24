#include <message.hpp>

namespace masonc
{
    void message_list::print_messages()
    {
         for (u64 i = 0; i < messages.size(); i += 1) {
            message* msg = &messages[i];

            if(msg->stage != build_stage::UNSET)
                std::cout << "[" << build_stage_name(msg->stage) << "] ";

            std::cout << "Message";

            if (msg->location.line_number > 0)
                std::cout << "(l. " << msg->location.line_number;

            if (msg->location.line_number > 0) {
                std::cout << "(l. " << msg->location.line_number;

                if (msg->location.start_column > 0 && msg->location.end_column > 0) {
                    if(msg->location.start_column == msg->location.end_column)
                        std::cout << ", c. " << msg->location.start_column;
                    else
                        std::cout << ", c. " << msg->location.start_column << "-" << msg->location.end_column;
                }

                std::cout << ")";
            }

            std::cout << ": " << msg->msg << std::endl;
        }
    }

    void message_list::print_warnings()
    {
        for (u64 i = 0; i < warnings.size(); i += 1) {
            message* warn = &warnings[i];

            if(warn->stage != build_stage::UNSET)
                std::cout << "[" << build_stage_name(warn->stage) << "] ";

            std::cout << "Warning";

            if (warn->location.line_number > 0) {
                std::cout << "(l. " << warn->location.line_number;

                if (warn->location.start_column > 0 && warn->location.end_column > 0) {
                    if(warn->location.start_column == warn->location.end_column)
                        std::cout << ", c. " << warn->location.start_column;
                    else
                        std::cout << ", c. " << warn->location.start_column << "-" << warn->location.end_column;
                }

                std::cout << ")";
            }

            std::cout << ": " << warn->msg << std::endl;
        }
    }

    void message_list::print_errors()
    {
        for (u64 i = 0; i < errors.size(); i += 1) {
            message* err = &errors[i];

            if(err->stage != build_stage::UNSET)
                std::cout << "[" << build_stage_name(err->stage) << "] ";

            std::cout << "Error";

            if (err->location.line_number > 0) {
                std::cout << "(l. " << err->location.line_number;

                if (err->location.start_column > 0 && err->location.end_column > 0) {
                    if(err->location.start_column == err->location.end_column)
                        std::cout << ", c. " << err->location.start_column;
                    else
                        std::cout << ", c. " << err->location.start_column << "-" << err->location.end_column;
                }

                std::cout << ")";
            }

            std::cout << ": " << err->msg << std::endl;
        }
    }

    void message_list::print()
    {
        print_messages();
        print_warnings();
        print_errors();

        std::cout << "\n";

        std::cout << messages.size() << " Messages" << std::endl;
        std::cout << warnings.size() << " Warnings" << std::endl;
        std::cout << errors.size() << " Errors" << std::endl;
        std::cout << std::endl;
    }

    void message_list::report_message(const std::string& msg, build_stage stage,
        const token_location& location)
    {
        messages.push_back(message{ location, msg, stage });
    }

    void message_list::report_warning(const std::string& msg, build_stage stage,
        const token_location& location)
    {
        warnings.push_back(message{ location, msg, stage });
    }

    void message_list::report_error(const std::string& msg, build_stage stage,
        const token_location& location)
    {
        errors.push_back(message{ location, msg, stage });
    }
}