#ifndef $_MASONC_PARSER_HPP_$
#define $_MASONC_PARSER_HPP_$

#include "common.hpp"
#include "lexer.hpp"
#include "symbol.hpp"
#include "scope.hpp"
#include "binary_operator.hpp"
#include "message.hpp"
#include "package.hpp"

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace masonc
{
	enum specifier : u8
	{
		SPECIFIER_NONE = 0,
		SPECIFIER_MUT = 1 << 0,
		SPECIFIER_CONST = 1 << 1
	};
	
	struct expression;

	// expression_parentheses
	// := "(" expression_binary ")"
	
	// specifiers
	// := (const | mut)

	// expression_literal
	// := expression_number_literal, expression_string etc.

	// argument_list
	// := variable_declaration ',' variable_declaration
	// := variable_declaration ',' argument_list
	
	// FIXME: Maybe change expression_unary.expr into a union of expression_primary and
	//		 expression_parentheses to avoid the pointer.
	
	// := op_code expression_primary | expression_parentheses
	struct expression_unary
	{
		expression* expr;
		s8 op_code;
	};
	
	// := expression op_code expression
	struct expression_binary
	{
		expression* left;
		expression* right;
		const binary_operator* op;
	};
	
	// := '(' expression_binary ')'
	struct expression_parentheses
	{
		expression_binary expr;
	};

	// := specifiers? name ':' '^'? type
	// := specifiers? name ':" '=' '&'? value (type inference)
	struct expression_variable_declaration
	{
		symbol name;
		std::string type_name;
		u8 specifiers;
		bool is_pointer;
	};
	
	// 'return_type' is nullptr if the procedure returns nothing (void)
	// := specifiers? 'proc' name '(' argument_list? ')' ('->' return_type)?
	struct expression_procedure_prototype
	{
		symbol name;
		std::vector<expression> argument_list;
		std::string return_type_name;
	};
	
	// := prototype '{' body '}'
	struct expression_procedure_definition
	{
		expression_procedure_prototype prototype;
		std::vector<expression> body;
	};
	
	// := name '(' argument_list? ')'
	struct expression_procedure_call
	{
		symbol name;
		std::vector<expression> argument_list;
	};

	enum number_type : u8
	{
		NUMBER_INTEGER,
		NUMBER_DECIMAL
	};

	struct expression_number_literal
	{
		number_type type;
		std::string value;
	};
	
	struct expression_string_literal
	{
		std::string value;
	};

	struct expression_reference
	{
		symbol name;
	};
	
	struct expression_package_declaration
	{
		std::string package_name;
	};
	
	struct expression_package_import
	{
		std::string package_name;
	};

	enum expression_type : u8
	{
		EXPR_EMPTY,
		EXPR_UNARY,
		EXPR_BINARY,
		EXPR_PARENTHESES,
		EXPR_NUMBER_LITERAL,
		EXPR_STRING_LITERAL,
		EXPR_REFERENCE,
		EXPR_VAR_DECLARATION,
		EXPR_PROC_PROTOTYPE,
		EXPR_PROC_DEFINITION,
		EXPR_PROC_CALL,
		EXPR_PACKAGE_DECLARATION,
		EXPR_PACKAGE_IMPORT
	};

	union expression_union
	{
		struct expression_empty_tagged { expression_type type; } empty;
		
		struct expression_unary_tagged { expression_type type; expression_unary value; } unary;
		struct expression_binary_tagged { expression_type type; expression_binary value; } binary;
		struct expression_parentheses_tagged { expression_type type; expression_parentheses value; } parentheses;
		struct expression_number_literal_tagged { expression_type type; expression_number_literal value; } number;
		struct expression_string_literal_tagged { expression_type type; expression_string_literal value; } str;
		struct expression_reference_tagged { expression_type type; expression_reference value; } reference;
		struct expression_variable_declaration_tagged { expression_type type; expression_variable_declaration value; } variable_declaration;
		struct expression_procedure_prototype_tagged { expression_type type; expression_procedure_prototype value; } procedure_prototype;
		struct expression_procedure_definition_tagged { expression_type type; expression_procedure_definition value; } procedure_definition;
		struct expression_procedure_call_tagged { expression_type type; expression_procedure_call value; } procedure_call;
		struct expression_package_declaration_tagged { expression_type type; expression_package_declaration value; } package_declaration;
		struct expression_package_import_tagged { expression_type type; expression_package_import value; } package_import;

		expression_union()
			: empty(expression_empty_tagged{ EXPR_EMPTY }) { }

        // TODO: Take argument as reference or something?
		expression_union(expression_unary value)
			: unary(expression_unary_tagged{ EXPR_UNARY, value }) { }
		
		expression_union(expression_binary value)
			: binary(expression_binary_tagged{ EXPR_BINARY, value }) { }
		
		expression_union(expression_parentheses value)
			: parentheses(expression_parentheses_tagged{ EXPR_PARENTHESES, value }) { }

		expression_union(expression_number_literal value)
			: number(expression_number_literal_tagged{ EXPR_NUMBER_LITERAL, value }) { }
		
		expression_union(expression_string_literal value)
			: str(expression_string_literal_tagged{ EXPR_STRING_LITERAL, value }) { }

		expression_union(expression_reference value)
			: reference(expression_reference_tagged{ EXPR_REFERENCE, value }) { }

		expression_union(expression_variable_declaration value)
			: variable_declaration(expression_variable_declaration_tagged{ EXPR_VAR_DECLARATION, value }) { }

		expression_union(expression_procedure_prototype value)
			: procedure_prototype(expression_procedure_prototype_tagged{ EXPR_PROC_PROTOTYPE, value }) { }

		expression_union(expression_procedure_definition value)
			: procedure_definition(expression_procedure_definition_tagged{ EXPR_PROC_DEFINITION, value }) { }

		expression_union(expression_procedure_call value)
			: procedure_call(expression_procedure_call_tagged{ EXPR_PROC_CALL, value }) { }
		
		expression_union(expression_package_declaration value)
			: package_declaration(expression_package_declaration_tagged{ EXPR_PACKAGE_DECLARATION, value }) { }
		
		expression_union(expression_package_import value)
			: package_import(expression_package_import_tagged{ EXPR_PACKAGE_IMPORT, value }) { }

		~expression_union() 
		{
			// This is safe because no matter the current variant, it is guaranteed to be at the same address (and type is the first member in tagged expression struct)
			switch (empty.type)
			{
			    default:
				    break;
			    case EXPR_EMPTY:
				    break;
			    case EXPR_UNARY:
				    unary.value.~expression_unary();
				    break;
			    case EXPR_BINARY:
				    binary.value.~expression_binary();
				    break;
			    case EXPR_PARENTHESES:
				    parentheses.value.~expression_parentheses();
				    break;
			    case EXPR_NUMBER_LITERAL:
				    number.value.~expression_number_literal();
				    break;
			    case EXPR_STRING_LITERAL:
				    str.value.~expression_string_literal();
				    break;
			    case EXPR_REFERENCE:
				    reference.value.~expression_reference();
				    break;
			    case EXPR_VAR_DECLARATION:
				    variable_declaration.value.~expression_variable_declaration();
				    break;
			    case EXPR_PROC_PROTOTYPE:
				    procedure_prototype.value.~expression_procedure_prototype();
				    break;
			    case EXPR_PROC_DEFINITION:
				    procedure_definition.value.~expression_procedure_definition();
				    break;
			    case EXPR_PROC_CALL:
				    procedure_call.value.~expression_procedure_call();
				    break;
			    case EXPR_PACKAGE_DECLARATION:
				    package_declaration.value.~expression_package_declaration();
				    break;
			    case EXPR_PACKAGE_IMPORT:
				    package_import.value.~expression_package_import();
				    break;
			}
		}

		expression_union(const expression_union& other)
		{
			// This is safe because no matter the current variant, it is guaranteed to be at the same address (and type is the first member in tagged expression struct)
			switch (other.empty.type)
			{
			    default:
				    break;
			    case EXPR_EMPTY:
				    new(&empty)expression_empty_tagged{};
				    break;
			    case EXPR_UNARY:
				    new(&empty)expression_unary_tagged{ EXPR_UNARY, other.unary.value };
				    break;
			    case EXPR_BINARY:
				    new(&empty)expression_binary_tagged{ EXPR_BINARY, other.binary.value };
				    break;
			    case EXPR_PARENTHESES:
				    new(&empty)expression_parentheses_tagged{ EXPR_PARENTHESES, other.parentheses.value };
				    break;
			    case EXPR_NUMBER_LITERAL:
				    new(&empty)expression_number_literal_tagged{ EXPR_NUMBER_LITERAL, other.number.value };
				    break;
			    case EXPR_STRING_LITERAL:
				    new(&empty)expression_string_literal_tagged{ EXPR_STRING_LITERAL, other.str.value };
				    break;
			    case EXPR_REFERENCE:
				    new(&empty)expression_reference_tagged{ EXPR_REFERENCE, other.reference.value };
				    break;
			    case EXPR_VAR_DECLARATION:
				    new(&empty)expression_variable_declaration_tagged{ EXPR_VAR_DECLARATION, other.variable_declaration.value };
				    break;
			    case EXPR_PROC_PROTOTYPE:
				    new(&empty)expression_procedure_prototype_tagged{ EXPR_PROC_PROTOTYPE, other.procedure_prototype.value };
				    break;
			    case EXPR_PROC_DEFINITION:
				    new(&empty)expression_procedure_definition_tagged{ EXPR_PROC_DEFINITION, other.procedure_definition.value };
				    break;
			    case EXPR_PROC_CALL:
				    new(&empty)expression_procedure_call_tagged{ EXPR_PROC_CALL, other.procedure_call.value };
				    break;
			    case EXPR_PACKAGE_DECLARATION:
				    new(&empty)expression_package_declaration_tagged{ EXPR_PACKAGE_DECLARATION, other.package_declaration.value };
				    break;
			    case EXPR_PACKAGE_IMPORT:
				    new(&empty)expression_package_import_tagged{ EXPR_PACKAGE_IMPORT, other.package_import.value };
				    break;
			}
		}

		expression_union& operator=(const expression_union& other)
		{
			// Destruct current variant
			this->~expression_union();

			// This is safe because no matter the current variant, it is guaranteed to be at the same address (and type is the first member in tagged expression struct)
			switch (other.empty.type)
			{
			    default:
				    break;
			    case EXPR_EMPTY:
				    new(&empty)expression_empty_tagged{};
				    break;
			    case EXPR_UNARY:
				    new(&empty)expression_unary_tagged{ EXPR_UNARY, other.unary.value };
				    break;
			    case EXPR_BINARY:
				    new(&empty)expression_binary_tagged{ EXPR_BINARY, other.binary.value };
				    break;
			    case EXPR_PARENTHESES:
				    new(&empty)expression_parentheses_tagged{ EXPR_PARENTHESES, other.parentheses.value };
				    break;
			    case EXPR_NUMBER_LITERAL:
				    new(&empty)expression_number_literal_tagged{ EXPR_NUMBER_LITERAL, other.number.value };
				    break;
			    case EXPR_REFERENCE:
				    new(&empty)expression_reference_tagged{ EXPR_REFERENCE, other.reference.value };
				    break;
			    case EXPR_VAR_DECLARATION:
				    new(&empty)expression_variable_declaration_tagged{ EXPR_VAR_DECLARATION, other.variable_declaration.value };
				    break;
			    case EXPR_PROC_PROTOTYPE:
				    new(&empty)expression_procedure_prototype_tagged{ EXPR_PROC_PROTOTYPE, other.procedure_prototype.value };
				    break;
			    case EXPR_PROC_DEFINITION:
				    new(&empty)expression_procedure_definition_tagged{ EXPR_PROC_DEFINITION, other.procedure_definition.value };
				    break;
			    case EXPR_PROC_CALL:
				    new(&empty)expression_procedure_call_tagged{ EXPR_PROC_CALL, other.procedure_call.value };
				    break;
			    case EXPR_PACKAGE_DECLARATION:
				    new(&empty)expression_package_declaration_tagged{ EXPR_PACKAGE_DECLARATION, other.package_declaration.value };
				    break;
			    case EXPR_PACKAGE_IMPORT:
				    new(&empty)expression_package_import_tagged{ EXPR_PACKAGE_IMPORT, other.package_import.value };
				    break;
			}

			return *this;
		}
	};

	struct expression
	{
		expression_union value;

		// Empty expression
		expression()
			: value()
		{ }

		expression(expression_union value)
			: value(value)
		{ }

		expression(const expression& other) = default;
		~expression() = default;
	};

	enum parse_context : u8
	{
		CONTEXT_NONE,
		CONTEXT_STATEMENT
	};
	
	// Package names associated with ASTs
	using package_ast_map = std::unordered_map<std::string, std::vector<expression>>;
	
	struct parser_output
	{
		package_map packages;
		
		// Each package defined in the parser's input gets its own AST.
		// After all parsers finished, ASTs of equal packages will be merged in the pre-linker.
		package_ast_map asts;
		
		message_list messages;
	};

	struct parser
	{
		// "input" is expected to have no errors,
		// "output" is expected to be allocated and empty.
		void parse(lexer_output* input, parser_output* output);
		
		void print_expressions();
		std::string format_expression(const expression& expr, u64 level = 0);
		
	private:
		lexer_output* input;
		parser_output* output;
		
		// "nullptr" when no package declaration was parsed.
		package* current_package;
		std::vector<expression>* current_ast;
		std::string current_package_name;
		
		scope_index current_scope_index;
		u64 token_index;

		bool done;
		
		// Drives the parser by parsing top-level expressions which
		// in turn parse their own expressions and so on.
		void drive();
		
		scope* current_scope();

		// Set package with specified name to be current, or add a new package if it doesn't exist
		void set_package(const std::string& package_name);

		void eat(u64 count = 1);
		std::optional<token*> peek_token();
		
		// If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
		std::optional<token*> expect_any();
		
        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        // 
        // If the next token is not an identifier, a parse error is generated and the result is empty.
        // 
        // Otherwise, the token is eaten and the result is valid.
		std::optional<token*> expect_identifier();
		
        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        // 
        // If the next token is not a character matching the argument,
        // a parse error is generated and the result is empty.
        // 
        // Otherwise, the token is eaten and the result is valid.
		std::optional<token*> expect(char c);
		
        // If a next token does not exist, the parser is marked as "done",
        // a parse error is generated and the result is empty.
        // 
        // If the next token is not an identifier matching the argument,
        // a parse error is generated and the result is empty.
        // 
        // Otherwise, the token is eaten and the result is valid.
		std::optional<token*> expect(const std::string& identifier);
		
		const std::string& identifier_at(const token& identifier_token);
		const std::string& integer_at(const token& integer_token);
		const std::string& decimal_at(const token& decimal_token);
		const std::string& string_at(const token& string_token);
		
		// Assumes that the index is in range.
		token_location* get_token_location(u64 token_index);
		
		// Report an error at the last token.
		void report_parse_error(const std::string& msg);
		
		// Report an error at a specific token.
		void report_parse_error_at(const std::string& msg, u64 token_index);
		
		// Jump to the next statement.
		void recover();
		
		// Guarantees that the next token exists and is an identifier, unless an error occured.
		std::optional<u8> parse_specifiers();
		
		// top-level expression
		// := expression_variable_declaration |
		//	  expression_procedure_prototype  |
		//	  expression_procedure_definition |
		//	  expression_package_declaration  |
		//	  expression_package_import
		std::optional<expression> parse_top_level();

		// statement (statements allowed in procedure bodies)
		// := expression_variable_declaration | 
		//    expression_procedure_call		  |
		//    assignment					  |
		//	  "return" expression
		std::optional<expression> parse_statement();
		
		// expression
		// := expression_primary | expression_binary

		// Parse primary or binary expressions
		std::optional<expression> parse_expression(parse_context context);
		
		// expression_primary
		// := expression_reference | expression_literal | expression_procedure_call |
		//    expression_unary
		std::optional<expression> parse_primary(parse_context context);

		// Parses right-hand side of a binary expression and returns the whole binary expression
		std::optional<expression> parse_binary(parse_context context,
			const expression& left, const binary_operator* op);
		
		// Parse either expression_primary which would ignore the parentheses,
		// or binary expression encased in parentheses, returning expression_parentheses
		std::optional<expression> parse_parentheses(parse_context context);
		
		std::optional<expression> parse_number_literal(parse_context context,
			const std::string& number, number_type type);

		std::optional<expression> parse_string_literal(parse_context context, const std::string& str);
		std::optional<expression> parse_reference(parse_context context, const std::string& identifier);

		std::optional<expression> parse_variable_declaration(parse_context context,
			const std::string& variable_name, u8 specifiers);
		
		std::optional<expression> parse_call(parse_context context, const std::string& procedure_name);
		std::vector<expression> parse_argument_list();

		// Returns either Expression_Procedure_Prototype or Expression_Procedure_Definition
		std::optional<expression> parse_procedure();
		std::optional<expression> parse_procedure_body(const expression_procedure_prototype& prototype);
		
		std::optional<expression> parse_package_declaration();
		std::optional<expression> parse_package_import();
	};
	
	// Returns `expression_binary` from either `expression_binary` or `expression_parentheses`.
	// Any other passed expression type will return a null pointer.
	expression_binary* get_binary_expression(expression* expr);
}

#endif