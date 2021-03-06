#include <build_stage.hpp>

namespace masonc
{
    const std::string build_stage_name(build_stage stage)
    {
        switch(stage)
        {
            default:
                return "";
            case build_stage::LEXER:
                return "Lexer";
            case build_stage::PARSER:
                return "Parser";
            case build_stage::LINKER:
                return "Linker";
            case build_stage::BYTECODE_GENERATOR:
                return "Bytecode Generator";
            case build_stage::CODE_GENERATOR:
                return "Code Generator";
        }
    }
}