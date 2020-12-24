#include <linker.hpp>

#include <parser_expressions.hpp>

#include <vector>
#include <optional>

namespace masonc::linker
{
    package* linker::package_from_name(const char* package_name)
    {
        auto search_package = parser_output()->package_names.find(package_name);

        if (!search_package)
            return nullptr;
        else
            return &parser_output()->packages[search_package.value()];
    }

    void linker::link(masonc::parser::parser_instance_output* parser_output, masonc::linker::linker_output* linker_output)
    {
        this->linker_output = linker_output;
        this->linker_output->parser_output = parser_output;

        for (u64 i = 0; i < parser_output->package_names.size(); i += 1) {
            const char* current_package_name = parser_output->package_names.at(i);
            package* current_package = &parser_output->packages[i];
            std::vector<masonc::parser::expression>* current_ast = &parser_output->asts[i];

            /*
            graph.add(current_package);

            for (u64 j = 0; j < current_package->package_import_names.size(); i += 1) {
                package* current_package_import = package_from_name(current_package->package_import_names.at(j));
                if (current_package_import == nullptr) {
                    // Error, the imported package could not be found.
                    report_link_error("No package declaration with name " + x + " found.");
                }

            }
            */
        }
    }

    masonc::parser::parser_instance_output* linker::parser_output()
    {
        return linker_output->parser_output;
    }

    void linker::report_link_error(const std::string& msg, const masonc::lexer::token_location& location)
    {
        this->linker_output->messages.report_error(msg, build_stage::LINKER, location);
    }
}