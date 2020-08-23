#include <binary_operator.hpp>

#include <optional>

namespace masonc
{
    std::optional<const binary_operator*> get_op(s8 op_code)
    {
        if (op_code == OP_EQUALS.op_code)
            return std::optional<const binary_operator*>{ &OP_EQUALS };
        if (op_code == OP_ADD.op_code)
            return std::optional<const binary_operator*>{ &OP_ADD };
        if (op_code == OP_SUB.op_code)
            return std::optional<const binary_operator*>{ &OP_SUB };
        if (op_code == OP_MUL.op_code)
            return std::optional<const binary_operator*>{ &OP_MUL };
        if (op_code == OP_DIV.op_code)
            return std::optional<const binary_operator*>{ &OP_DIV };

        return std::optional<const binary_operator*>{};
    }
}