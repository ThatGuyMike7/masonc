#include <linker.hpp>

#include <parser_expressions.hpp>

#include <vector>
#include <optional>

namespace masonc::linker
{
    mod* linker::module_from_name(const char* module_name)
    {
        /*
        auto search_module = parser_output()->module_names.find(module_name);

        if (!search_module)
            return nullptr;
        else
            return &parser_output()->modules[search_module.value()];
        */
        return nullptr;
    }

    void linker::link(std::vector<masonc::parser::parser_instance_output>* parser_outputs,
        masonc::linker::linker_output* linker_output)
    {
        this->linker_output = linker_output;
        this->linker_output->parser_outputs = parser_outputs;

        /*
        for (u64 i = 0; i < parser_output->module_names.size(); i += 1) {
            const char* current_module_name = parser_output->module_names.at(i);
            mod* current_module = &parser_output->modules[i];
            std::vector<masonc::parser::expression>* current_ast = &parser_output->asts[i];

            graph.add(current_module);

            for (u64 j = 0; j < current_module->module_import_names.size(); i += 1) {
                mod* current_module_import = module_from_name(current_module->module_import_names.at(j));
                if (current_module_import == nullptr) {
                    // Error, the imported module could not be found.
                    report_link_error("No module declaration with name " + x + " found.");
                }

            }
        }
        */
    }

    /*
    masonc::parser::parser_instance_output* linker::parser_output()
    {
        return linker_output->parser_output;
    }
    */

    void linker::report_link_error(const std::string& msg, const masonc::lexer::token_location& location)
    {
        this->linker_output->messages.report_error(msg, build_stage::LINKER, location);
    }
}