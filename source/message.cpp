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

            if (msg->line_number > 0)
                std::cout << "(l. " << msg->line_number;

            if (msg->line_number > 0) {
                std::cout << "(l. " << msg->line_number;

                if (msg->start_column > 0 && msg->end_column > 0) {
                    if(msg->start_column == msg->end_column)
                        std::cout << ", c. " << msg->start_column;
                    else
                        std::cout << ", c. " << msg->start_column << "-" << msg->end_column;
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

            if (warn->line_number > 0) {
                std::cout << "(l. " << warn->line_number;

                if (warn->start_column > 0 && warn->end_column > 0) {
                    if(warn->start_column == warn->end_column)
                        std::cout << ", c. " << warn->start_column;
                    else
                        std::cout << ", c. " << warn->start_column << "-" << warn->end_column;
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

            if (err->line_number > 0) {
                std::cout << "(l. " << err->line_number;

                if (err->start_column > 0 && err->end_column > 0) {
                    if(err->start_column == err->end_column)
                        std::cout << ", c. " << err->start_column;
                    else
                        std::cout << ", c. " << err->start_column << "-" << err->end_column;
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

    void message_list::report_message(const std::string& msg, build_stage stage, u64 line_number,
        u64 start_column, u64 end_column)
    {
        messages.push_back(message{ line_number, start_column, end_column, msg, stage });
    }

    void message_list::report_warning(const std::string& msg, build_stage stage, u64 line_number,
        u64 start_column, u64 end_column)
    {
        warnings.push_back(message{ line_number, start_column, end_column, msg, stage });
    }

    void message_list::report_error(const std::string& msg, build_stage stage, u64 line_number,
        u64 start_column, u64 end_column)
    {
        errors.push_back(message{ line_number, start_column, end_column, msg, stage });
    }
}