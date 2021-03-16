#include <linker.hpp>

#include <parser_expressions.hpp>
#include <mod_handle.hpp>
#include <build_stage.hpp>

#include <vector>
#include <optional>
#include <algorithm>

namespace masonc::linker
{
    /*
    mod* linker::module_from_name(const char* module_name)
    {
        using namespace masonc::parser;

        for (u64 i = 0; i < linker_output->parser_outputs->size(); i += 1) {
            parser_instance_output* parser_output = &(*linker_output->parser_outputs)[i];
            std::optional<mod_handle> search_module = parser_output->module_names.find(module_name);

            if (search_module)
                return &parser_output->modules[search_module.value()];
        }

        return nullptr;
    }
    */

    void linker::link(std::vector<masonc::parser::parser_instance_output>* parser_outputs,
        masonc::linker::linker_output* linker_output)
    {
        this->linker_output = linker_output;
        this->linker_output->parser_outputs = parser_outputs;
    }

    void linker::report_link_error(const std::string& msg, const masonc::lexer::token_location& location)
    {
        this->linker_output->messages.report_error(msg, build_stage::LINKER, location);
    }
}