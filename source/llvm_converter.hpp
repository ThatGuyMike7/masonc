#ifndef MASONC_LLVM_CONVERTER_HPP
#define MASONC_LLVM_CONVERTER_HPP

#include <parser.hpp>
#include <message.hpp>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <robin_hood.hpp>

#include <string>

namespace masonc::llvm
{
    inline robin_hood::unordered_map<const char*, LLVMTypeRef> type_map;

    void initialize_llvm_converter();

    // A term element is either a primary expression, a binary operator token, a parenthesis token
    // or an evaluated LLVM value reference.
    struct term_element
    {
        enum
        {
            VARIANT_PRIMARY,
            VARIANT_OP,
            VARIANT_PARENTHESIS_BEGIN,
            VARIANT_PARENTHESIS_END,
            VARIANT_LLVM_VALUE
        } type;

        union
        {
            masonc::parser::expression* expr;
            const binary_operator* op;
            LLVMValueRef llvm_value;
        };
    };

    struct llvm_converter_output
    {
        // Types of declared functions associated with their names.
        robin_hood::unordered_map<std::string, LLVMTypeRef> function_type_map;

        LLVMModuleRef llvm_module;
        message_list messages;
    };

    // The struct layout models the layout of the parser closely.
    // See the parser header for documentation about the terminology
    // and what each function generates.
    //
    // Many converter functions can return "nullptr" on error,
    // though that should not be happening if the parser finished successfully.
    //
    // "llvm_converter" is responsible for IR generation of a specific module.
    struct llvm_converter
    {
        void convert(masonc::lexer::lexer_instance_output* input_lexer, masonc::parser::parser_instance_output* input_parser,
            llvm_converter_output* output);

        void free();

        void print_IR();

    private:
        masonc::lexer::lexer_instance_output* input_lexer;
        masonc::parser::parser_instance_output* input_parser;
        llvm_converter_output* output;

        LLVMBuilderRef llvm_builder;
        LLVMBuilderRef llvm_sub_builder;

        // TODO: Add stuff here.
        void add_built_in_procedures();

        // Returns 'nullptr' if type was not found.
        LLVMTypeRef llvm_type_by_name(const char* type_name);
        LLVMTypeRef llvm_pointer_type(LLVMTypeRef llvm_element_type);

        LLVMValueRef build_alloca_at_entry(LLVMValueRef llvm_function,
            LLVMTypeRef llvm_variable_type, const char* variable_name);

        LLVMValueRef convert_top_level(masonc::parser::expression* expr);
        LLVMValueRef convert_statement(masonc::parser::expression* expr, LLVMValueRef llvm_function);

        LLVMValueRef convert_expression(masonc::parser::expression* expr);
        LLVMValueRef convert_primary(masonc::parser::expression* expr);

        LLVMValueRef convert_number_literal(masonc::parser::expression_number_literal* expr);

        LLVMValueRef convert_local_variable(masonc::parser::expression_variable_declaration* expr,
            LLVMValueRef llvm_function);

        LLVMValueRef convert_reference(masonc::parser::expression_reference* expr);

        LLVMValueRef convert_reference_of(LLVMValueRef llvm_value, LLVMValueRef llvm_pointer);
        LLVMValueRef convert_dereference(masonc::parser::expression* expr);

        // TODO: Implement this
        LLVMValueRef convert_global_variable(masonc::parser::expression_variable_declaration* expr);

        LLVMValueRef convert_call(masonc::parser::expression_procedure_call* expr);

        LLVMValueRef convert_procedure(masonc::parser::expression_procedure_definition* expr);
        LLVMValueRef convert_procedure_prototype(masonc::parser::expression_procedure_prototype* expr);

        void convert_procedure_body(LLVMValueRef llvm_function,
            masonc::parser::expression_procedure_definition* expr);

        LLVMValueRef convert_binary(s8 op_code, LLVMValueRef left, LLVMValueRef right);
        LLVMValueRef convert_term(masonc::parser::expression* term_start);

        // Transform a tree of binary operations into a list that resembles infix notation.
        void ast_to_infix(masonc::parser::expression* term_start, std::vector<term_element>& term);

        // Transform a term from infix notation to reverse polish notation (shunting yard algorithm).
        std::vector<term_element> infix_to_RPN(const std::vector<term_element>& term);

        void print_term(const std::vector<term_element>& term);
    };
}

#endif