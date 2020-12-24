#include <llvm_converter.hpp>

#include <log.hpp>
#include <type.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <limits>

namespace masonc::llvm
{
    void initialize_llvm_converter()
    {
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_VOID, LLVMVoidType() });

        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_BOOL, LLVMInt1Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_CHAR, LLVMInt8Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_U8, LLVMInt8Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_S8, LLVMInt8Type() });

        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_U16, LLVMInt16Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_S16, LLVMInt16Type() });

        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_U32, LLVMInt32Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_S32, LLVMInt32Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_F32, LLVMFloatType() });

        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_U64, LLVMInt64Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_S64, LLVMInt64Type() });
        type_map.insert(robin_hood::pair<const char*, LLVMTypeRef>{ TYPE_F64, LLVMDoubleType() });
    }

    void llvm_converter::convert(masonc::lexer::lexer_instance_output* input_lexer, masonc::parser::parser_instance_output* input_parser,
        llvm_converter_output* output)
    {
        this->input_lexer = input_lexer;
        this->input_parser = input_parser;
        this->output = output;
        output->llvm_module = LLVMModuleCreateWithName("main_module");

        llvm_builder = LLVMCreateBuilder();
        llvm_sub_builder = LLVMCreateBuilder();

        add_built_in_procedures();

        //for (size_t i = 0; i < input_parser->; i += 1) {
        //    expression* expr = &input->expressions[i];
        //    convert_top_level(expr);
        //}

        //char* msg = LLVMCreateMessage("");
        //LLVMBool llvm_verify = LLVMVerifyModule(output->llvm_module, LLVMPrintMessageAction, &msg);

        //LLVMDisposeMessage(msg);
    }

    void llvm_converter::free()
    {
        LLVMDisposeBuilder(llvm_sub_builder);
        LLVMDisposeBuilder(llvm_builder);
        LLVMDisposeModule(output->llvm_module);
    }

    void llvm_converter::print_IR()
    {
        char* str = LLVMPrintModuleToString(output->llvm_module);
        std::cout << std::endl << str << std::endl;
        LLVMDisposeMessage(str);
    }

    void llvm_converter::add_built_in_procedures()
    {
    }

    LLVMTypeRef llvm_converter::llvm_type_by_name(const char* type_name)
    {
        const auto find_it = type_map.find(type_name);
        if (find_it == type_map.end())
            return nullptr;

        return find_it->second;
    }

    LLVMTypeRef llvm_converter::llvm_pointer_type(LLVMTypeRef llvm_element_type)
    {
        return LLVMPointerType(llvm_element_type, 0);
    }

    LLVMValueRef llvm_converter::build_alloca_at_entry(LLVMValueRef llvm_function,
        LLVMTypeRef llvm_variable_type, const char* variable_name)
    {
        LLVMBasicBlockRef llvm_function_entry_block = LLVMGetEntryBasicBlock(llvm_function);

        // Position the sub-builder at the start of the function's entry block.
        LLVMValueRef llvm_entry_first_instruction = LLVMGetFirstInstruction(llvm_function_entry_block);
        LLVMPositionBuilder(llvm_sub_builder, llvm_function_entry_block, llvm_entry_first_instruction);

        return LLVMBuildAlloca(llvm_sub_builder, llvm_variable_type, variable_name);
    }

    LLVMValueRef llvm_converter::convert_top_level(masonc::parser::expression* expr)
    {
        switch (expr->value.empty.type) {
            default:
                log_error(
                    std::string{ "Cannot generate code for expression of type " +
                    std::to_string(expr->value.empty.type) }.c_str()
                );
                return nullptr;
            case masonc::parser::EXPR_VAR_DECLARATION:
                return nullptr;
            case masonc::parser::EXPR_PROC_PROTOTYPE:
                return convert_procedure_prototype(&expr->value.procedure_prototype.value);
            case masonc::parser::EXPR_PROC_DEFINITION:
                return convert_procedure(&expr->value.procedure_definition.value);
        }
    }

    LLVMValueRef llvm_converter::convert_statement(masonc::parser::expression* expr, LLVMValueRef llvm_function)
    {
        if(expr->value.empty.type == masonc::parser::EXPR_VAR_DECLARATION)
            return convert_local_variable(&expr->value.variable_declaration.value, llvm_function);
        else if(expr->value.empty.type == masonc::parser::EXPR_PROC_CALL)
            return convert_call(&expr->value.procedure_call.value);

        log_error(
            std::string{ "Cannot generate code for expression of type " +
            std::to_string(expr->value.empty.type) }.c_str()
        );

        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_expression(masonc::parser::expression* expr)
    {
        if (expr->value.empty.type == masonc::parser::EXPR_BINARY ||
            expr->value.empty.type == masonc::parser::EXPR_PARENTHESES)
        {
            return convert_term(expr);
        }

        return convert_primary(expr);
    }

    LLVMValueRef llvm_converter::convert_primary(masonc::parser::expression* expr)
    {
        /*
        switch (expr->value.empty.type) {
            default:
                return nullptr;
            case EXPR_UNARY:
                switch(expr->value.unary.value.op_code) {
                    default:
                        // TODO: Report error.
                        return nullptr;
                    case '&':
                        return convert_reference_of(expr->value.unary.value.expr);
                    case '^':
                        return convert_dereference(expr->value.unary.value.expr);
                }
            case EXPR_REFERENCE:
                return convert_reference(&expr->value.reference.value);
            case EXPR_NUMBER_LITERAL:
                return convert_number_literal(&expr->value.number.value);
            case EXPR_STRING_LITERAL:
                // TODO: Implement string literal.
                return nullptr;
            case EXPR_PROC_CALL:
                return convert_call(&expr->value.procedure_call.value);
        }
        */
        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_number_literal(masonc::parser::expression_number_literal* expr)
    {
        /*
        switch (expr->type) {
            default:
                // TODO: Report error.
                return nullptr;
            case NUMBER_INTEGER:
                // Const integer literals have 64 bit precision.
                return LLVMConstIntOfStringAndSize(
                    LLVMInt64Type(),
                    expr->value,
                    static_cast<unsigned int>(expr->value_length),
                    10u
                );
            case NUMBER_DECIMAL:
                // Const decimal literals have 64 bit precision.
                return LLVMConstRealOfStringAndSize(
                    LLVMDoubleType(),
                    expr->value,
                    static_cast<unsigned int>(expr->value_length)
                );
        }
        */
        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_local_variable(masonc::parser::expression_variable_declaration* expr,
        LLVMValueRef llvm_function)
    {
        /*
        LLVMTypeRef llvm_variable_type;

        if(expr->is_pointer)
            llvm_variable_type = llvm_pointer_type(llvm_type_by_name(expr->));
        else
            llvm_variable_type = llvm_type_by_name(expr->type_name);

        return build_alloca_at_entry(llvm_function, llvm_variable_type, expr->name.name.c_str());
        */
        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_reference(masonc::parser::expression_reference* expr)
    {
        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_reference_of(LLVMValueRef llvm_value,
        LLVMValueRef llvm_pointer)
    {
        return LLVMBuildStore(llvm_builder, llvm_value, llvm_pointer);
    }

    LLVMValueRef llvm_converter::convert_dereference(masonc::parser::expression* expr)
    {
        /*
        return LLVMBuildLoad2(llvm_builder,
            LLVMTypeRef Ty, LLVMValueRef PointerVal, const char *Name);
            */
        return nullptr;
    }

    LLVMValueRef llvm_converter::convert_call(masonc::parser::expression_procedure_call* expr)
    {
        /*
        LLVMValueRef llvm_function = LLVMGetNamedFunction(output->llvm_module, expr->name.name.c_str());

        // No error checking because the linker should have figured out already if
        // the function is visible from here or not.
        LLVMTypeRef llvm_function_type = output->function_type_map[expr->name.name];

        u64 args_count = expr->argument_list.size();
        LLVMValueRef* args = static_cast<LLVMValueRef*>(alloca(sizeof(LLVMValueRef) * args_count));

        for(u64 i = 0; i < args_count; i += 1) {
            args[i] = convert_expression(&expr->argument_list[i]);
        }

        // Overflow check before casting `args_count` from `u64` to `unsigned int`
        std::numeric_limits<unsigned int> unsigned_int_limit;
        assume(unsigned_int_limit.max() >= args_count);

        return LLVMBuildCall2(
            llvm_builder,
            llvm_function_type,
            llvm_function,
            args,
            static_cast<unsigned int>(args_count),
            ""
        );
        */
       return nullptr;
    }

    LLVMValueRef llvm_converter::convert_procedure(masonc::parser::expression_procedure_definition* expr)
    {
        LLVMValueRef llvm_function = convert_procedure_prototype(&expr->prototype);
        convert_procedure_body(llvm_function, expr);

        return llvm_function;
    }

    LLVMValueRef llvm_converter::convert_procedure_prototype(masonc::parser::expression_procedure_prototype* expr)
    {
        /*
        std::vector<LLVMTypeRef> llvm_argument_types;

        for (u64 i = 0; i < expr->argument_list.size(); i += 1)
        {
            expression_variable_declaration* arg =
                &expr->argument_list[i].value.variable_declaration.value;

            LLVMTypeRef llvm_arg_type = llvm_type_by_name(arg->type_name);
            if (llvm_arg_type == nullptr)
            {
                log_error(
                    std::string{ "Argument type '" + arg->type_name +
                    "' could not be found" }.c_str()
                );

                return nullptr;
            }

            llvm_argument_types.push_back(llvm_arg_type);
        }

        LLVMTypeRef llvm_return_type = llvm_type_by_name(expr->return_type_name);
        if (llvm_return_type == nullptr)
        {
            log_error(
                std::string{ "Return type '" + expr->return_type_name +
                "' could not be found" }.c_str()
            );

            return nullptr;
        }

        // Overflow check before casting "llvm_argument_types.size()" from "size_t" to "unsigned int".
        std::numeric_limits<unsigned int> unsigned_int_limit;
        assume(unsigned_int_limit.max() >= llvm_argument_types.size());

        LLVMTypeRef llvm_function_type = LLVMFunctionType(
            llvm_return_type,
            llvm_argument_types.data(),
            static_cast<unsigned int>(llvm_argument_types.size()),
            false
        );

        output->function_type_map.insert(std::make_pair(expr->name.name, llvm_function_type));

        return LLVMAddFunction(output->llvm_module, expr->name.name.c_str(), llvm_function_type);
        */
       return nullptr;
    }

    void llvm_converter::convert_procedure_body(LLVMValueRef llvm_function,
        masonc::parser::expression_procedure_definition* expr)
    {
        LLVMBasicBlockRef llvm_function_block = LLVMAppendBasicBlock(llvm_function, "entry");
        LLVMPositionBuilderAtEnd(llvm_builder, llvm_function_block);

        // Generate IR for all statements in the procedure's body.
        for(size_t i = 0; i < expr->body.size(); i += 1) {
            convert_statement(&expr->body[i], llvm_function);
        }

        // Generate terminator for basic block.
        LLVMValueRef llvm_return = LLVMBuildRetVoid(llvm_builder);

        assume(LLVMVerifyFunction(llvm_function, LLVMPrintMessageAction),
            "LLVM function verification failed.");
    }

    LLVMValueRef llvm_converter::convert_binary(s8 op_code, LLVMValueRef left, LLVMValueRef right)
    {
        // TODO: Type checking
        switch(op_code) {
            default:
                // TODO: Report error
                return nullptr;
            case '=':

                break;
            case '+':
                return LLVMBuildAdd(llvm_builder, left, right, "addtmp");
            case '-':
                return LLVMBuildSub(llvm_builder, left, right, "subtmp");
            case '*':
                return LLVMBuildMul(llvm_builder, left, right, "multmp");
            case '/':
                return LLVMBuildSDiv(llvm_builder, left, right, "divtmp");
        }
    }

    LLVMValueRef llvm_converter::convert_term(masonc::parser::expression* term_start)
    {
        std::vector<term_element> infix;
        std::vector<term_element> RPN;

        ast_to_infix(term_start, infix);
        RPN = infix_to_RPN(infix);

        while(RPN.size() > 1) {
            for(u64 i = 0; i < RPN.size(); i += 1) {
                term_element& element = RPN[i];

                if(element.type == term_element::VARIANT_OP) {
                    term_element& left = RPN[i - 2];
                    term_element& right = RPN[i - 1];

                    LLVMValueRef llvm_left;
                    LLVMValueRef llvm_right;

                    // Operands either have to be evaluated or already have been evaluated.
                    if(left.type == term_element::VARIANT_PRIMARY)
                        llvm_left = convert_primary(left.expr);
                    else
                        llvm_left = left.llvm_value;

                    if(right.type == term_element::VARIANT_PRIMARY)
                        llvm_right = convert_primary(right.expr);
                    else
                        llvm_right = right.llvm_value;

                    // Evaluate binary operation.
                    LLVMValueRef llvm_binary_result = convert_binary(element.op->op_code,
                        llvm_left, llvm_right);

                    // Change the variant of the operator to our LLVM value result and
                    // remove the now evaluated left and right operands.
                    element.type = term_element::VARIANT_LLVM_VALUE;
                    element.llvm_value = llvm_binary_result;
                    RPN.erase(RPN.begin() + i - 2, RPN.begin() + i);

                    break;
                }
            }
        }

        // The last remaining element is the result of the chain of binary operations.
        return RPN[0].llvm_value;
    }

    void llvm_converter::ast_to_infix(masonc::parser::expression* term_start, std::vector<term_element>& term)
    {
        masonc::parser::expression_binary* expr;

        if(term_start->value.empty.type == masonc::parser::EXPR_BINARY) {
            expr = &term_start->value.binary.value;

            // Left-hand side
            ast_to_infix(expr->left, term);

            // Operator
            term_element op;
            op.type = term_element::VARIANT_OP;
            op.op = expr->op;
            term.push_back(op);

            // Right-hand side
            ast_to_infix(expr->right, term);
        }
        else if (term_start->value.empty.type == masonc::parser::EXPR_PARENTHESES) {
            expr = &term_start->value.parentheses.value.expr;

            // "("
            term_element parenthesis_begin;
            parenthesis_begin.type = term_element::VARIANT_PARENTHESIS_BEGIN;
            term.push_back(parenthesis_begin);

            // Left-hand side
            ast_to_infix(expr->left, term);

            // Operator
            term_element op;
            op.type = term_element::VARIANT_OP;
            op.op = expr->op;
            term.push_back(op);

            // Right-hand side
            ast_to_infix(expr->right, term);

            // ")"
            term_element parenthesis_end;
            parenthesis_end.type = term_element::VARIANT_PARENTHESIS_END;
            term.push_back(parenthesis_end);
        }
        else {
            term_element primary;
            primary.type = term_element::VARIANT_PRIMARY;
            primary.expr = term_start;
            term.push_back(primary);
        }
    }

    std::vector<term_element> llvm_converter::infix_to_RPN(
        const std::vector<term_element>& term)
    {
        // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
        // https://en.wikipedia.org/wiki/Reverse_Polish_notation

        std::vector<term_element> output_queue;
        std::vector<term_element> operator_stack;

        for(u64 i = 0; i < term.size(); i += 1) {
            const term_element& element = term[i];

            if(element.type == term_element::VARIANT_PRIMARY) {
                output_queue.push_back(element);
            }
            else if(element.type == term_element::VARIANT_OP) {
                while(operator_stack.size() > 0 &&
                      operator_stack.back().type != term_element::VARIANT_PARENTHESIS_BEGIN &&
                      operator_stack.back().op->precedence >= element.op->precedence)
                {
                    output_queue.push_back(operator_stack.back());
                    operator_stack.pop_back();
                }

                operator_stack.push_back(element);
            }
            else if(element.type == term_element::VARIANT_PARENTHESIS_BEGIN) {
                operator_stack.push_back(element);
            }
            else if(element.type == term_element::VARIANT_PARENTHESIS_END) {
                u64 j = operator_stack.size() - 1;

                while(operator_stack[j].type != term_element::VARIANT_PARENTHESIS_BEGIN) {
                    output_queue.push_back(operator_stack[j]);
                    operator_stack.pop_back();

                    j -= 1;
                    //if(j < 0) {
                        // Mismatched parentheses
                    //}
                }

                // Discard the left parenthesis.
                operator_stack.pop_back();
            }
        }

        // Push remaining operators onto the output queue.
        for(u64 i = 0; i < operator_stack.size(); i += 1) {
            output_queue.push_back(operator_stack[i]);
        }

        return output_queue;
    }

    void llvm_converter::print_term(const std::vector<term_element>& term)
    {
        std::cout << std::endl;
        for (u64 i = 0; i < term.size(); i += 1) {
            const term_element& element = term[i];
            switch (element.type) {
                case term_element::VARIANT_OP:
                    std::cout << static_cast<char>(element.op->op_code) << " ";
                    break;
                case term_element::VARIANT_PRIMARY:
                    std::cout << element.expr->value.number.value.value << " ";
                    break;
                case term_element::VARIANT_PARENTHESIS_BEGIN:
                    std::cout << "( ";
                    break;
                case term_element::VARIANT_PARENTHESIS_END:
                    std::cout << ") ";
                    break;
                case term_element::VARIANT_LLVM_VALUE:
                    std::cout << "LLVMValueRef ";
                    break;
            }
        }
        std::cout << std::endl;
    }
}