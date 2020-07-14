#ifndef $_MASON_LLVM_CONVERTER_HPP_$
#define $_MASON_LLVM_CONVERTER_HPP_$

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

#include <string>
#include <map>

#include "parser.hpp"

namespace masonc
{
    inline std::map<std::string, LLVMTypeRef> type_map;

    void initialize_llvm_converter();
    
    // A term element is either a primary expression, a binary operator token, a parenthesis token
    // or an evaluated LLVM value reference
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
            expression* expr;
            const binary_operator* op;
            LLVMValueRef llvm_value;
        };
    };
    
    // The struct layout models the layout of the parser closely.
    // See the parser header for documentation about the terminology
    // and what each function generates.
    //
    // Many converter functions can return `nullptr` on error,
    // though that should not be happening if the parser finished successfully.
    struct llvm_converter
    {
        void generate(parser_output* input);
        void free();
        
        void print_IR();
        
    private:
        parser_output* input;
        
        // The types of declared functions will be stashed here, associated with their name
        std::map<std::string, LLVMTypeRef> function_type_map;

        LLVMModuleRef llvm_module;
        LLVMBuilderRef llvm_builder;
        LLVMBuilderRef llvm_sub_builder;
        
        // TODO: Add stuff here
        void add_built_in_procedures();
        
        // Returns 'nullptr' if type was not found
        LLVMTypeRef get_llvm_type_by_name(const std::string& type_name);
        LLVMTypeRef get_llvm_pointer_type(LLVMTypeRef llvm_element_type);
        
        LLVMValueRef build_alloca_at_entry(LLVMValueRef llvm_function,
            LLVMTypeRef llvm_variable_type, const char* variable_name);
        
        LLVMValueRef convert_top_level(expression* expr);
        LLVMValueRef convert_statement(expression* expr, LLVMValueRef llvm_function);
        
        LLVMValueRef convert_expression(expression* expr);
        LLVMValueRef convert_primary(expression* expr);
        
        LLVMValueRef convert_number_literal(expression_number_literal* expr);
        
        LLVMValueRef convert_local_variable(expression_variable_declaration* expr,
            LLVMValueRef llvm_function);
        
        LLVMValueRef convert_reference(expression_reference* expr);
        
        LLVMValueRef convert_reference_of(LLVMValueRef llvm_value, LLVMValueRef llvm_pointer);
        LLVMValueRef convert_dereference(expression* expr);
        
        // TODO: Implement this
        LLVMValueRef convert_global_variable(expression_variable_declaration* expr);
        
        LLVMValueRef convert_call(expression_procedure_call* expr);
        
        LLVMValueRef convert_procedure(expression_procedure_definition* expr);
        LLVMValueRef convert_procedure_prototype(expression_procedure_prototype* expr);
        
        void convert_procedure_body(LLVMValueRef llvm_function,
            expression_procedure_definition* expr);
        
        LLVMValueRef convert_binary(s8 op_code, LLVMValueRef left, LLVMValueRef right);
        LLVMValueRef convert_term(expression* term_start);
        
        // Transform a tree of binary operations into a list that resembles infix notation
        void AST_to_infix(expression* term_start, std::vector<term_element>& term);
        
        // Transform a term from infix notation to reverse polish notation (shunting yard algorithm)
        std::vector<term_element> infix_to_RPN(const std::vector<term_element>& term);
        
        void print_term(const std::vector<term_element>& term);
    };
}

#endif