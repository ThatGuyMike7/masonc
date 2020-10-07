#include <linker.hpp>

namespace masonc
{
    package* linker::package_from_name(const char* package_name)
    {
        auto search_package = parser_output()->package_names.find(package_name);

        if (!search_package)
            return nullptr;
        else
            return &parser_output()->packages[search_package.value()];
    }

    void linker::link(masonc::parser_output* parser_output, masonc::linker_output* linker_output)
    {
        this->linker_output = linker_output;
        this->linker_output->parser_output = parser_output;


    }

    parser_output* linker::parser_output()
    {
        return linker_output->parser_output;
    }
}