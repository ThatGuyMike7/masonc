#include "parser.hpp"

#include "lang.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "lexer.hpp"

#include <iostream>

namespace masonc
{
	void parser::set_current_package(const std::string& package_name)
	{
		for (u64 i = 0; i < packages.size(); i += 1)
		{
			if (packages[i].package_name == package_name)
			{
				current_package_index = i;
				return;
			}
		}

		current_package_index = packages.size();
		package new_package = { package_name };
		packages.push_back(new_package);
	}
	
	const std::string* parser::current_package_name()
	{
		if (current_package_index < 0)
			return &GLOBAL_PACKAGE_NAME;//const_cast<std::string*>(&GLOBAL_PACKAGE_NAME);

		return &packages[current_package_index].package_name;
	}

	void parser::recover()
	{
		while(true)
		{
			result<token*> token_result = get_token();
			if (!token_result)
			{
				return;
			}

			if (token_result.value()->type == ';' || 
			   (token_result.value()->type == TOKEN_IDENTIFIER &&
				input->identifiers[token_result.value()->value_index] == "proc"))
			{
				return;
			}
		}
	}

	result<token*> parser::get_token()
	{
		if(token_index < input->tokens.size())
		{
			result<token*> result{ &input->tokens[token_index] };
			token_index += 1;
			return result;
		}
		
		done = true;
		return result<token*>{};
	}
	
	result<token*> parser::peek_token()
	{
		if(token_index < input->tokens.size())
		{
			result<token*> result{ &input->tokens[token_index] };
			return result;
		}
		
		return result<token*>{};
	}
	
	token_location* parser::get_token_location(u64 token_index)
	{
		return &input->locations[token_index];
	}
	
	void parser::report_parse_error(const std::string& msg, u64 token_index)
	{
		token_location* location = get_token_location(token_index);
		
		output->messages.report_error(msg, build_stage::PARSER,
			location->line_number, location->start_column, location->end_column
		);
	}
	
	void parser::parse(lexer_output* input, parser_output* output, scope* global_scope_template)
	{
		this->input = input;
		this->output = output;
		this->global_scope = *global_scope_template;
		this->current_scope_index = global_scope.index;
		
		this->current_package_index = -1;
		this->token_index = 0;
		this->done = false;

		try
		{
			// Guess how many tokens will end up being 1 expression on average to
			// avoid reallocations.
			output->expressions.reserve(input->tokens.size() / 10 + 32);
		}
		catch (...)
		{
			log_warning("Could not reserve space for token vector");
		}

		while(true)
		{
			// Reached the end of the token stream
			result<token*> token_result = peek_token();
			if (!token_result)
				break;

			expression e = parse_top_level();

			// Early out, probably due to a parse error
			if (done)
				break;

			output->expressions.push_back(e);
		}
	}
	
	result<u8> parser::parse_specifiers()
	{
		u8 specifiers = SPECIFIER_NONE;
		
		while(true)
		{
			result<token*> token_result = peek_token();
			if(!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				std::cout << "\n\nHA\n\n";
				return result<u8>{};
			}
			
			if(token_result.value()->type != TOKEN_IDENTIFIER)
			{
				report_parse_error("Unexpected token", this->token_index - 1);
				
				recover();
				return result<u8>{};
			}
			
			const std::string& identifier = input->identifiers[token_result.value()->value_index];
			
			if(identifier == "mut")
			{
				// Eat the specifier
				get_token();
				
				if(specifiers & SPECIFIER_MUT)
				{
					report_parse_error("Redundant specifier", this->token_index - 1);
					continue;
				}
				
				specifiers |= SPECIFIER_MUT;
			}
			else if(identifier == "const")
			{
				// Eat the specifier
				get_token();
				
				if(specifiers & SPECIFIER_CONST)
				{
					report_parse_error("Redundant specifier", this->token_index - 1);
					continue;
				}
				
				specifiers |= SPECIFIER_CONST;
			}
			else
			{
				return result<u8>{ specifiers };
			}
		}
	}
	
	expression parser::parse_top_level()
	{
		result<u8> specifiers_result = parse_specifiers();
		if(!specifiers_result)
			return expression{};
		
		// Next token is guaranteed to be an identifier
		
		result<token*> token_result = get_token();
		const std::string& identifier = input->identifiers[token_result.value()->value_index];
		
		if(specifiers_result.value() == SPECIFIER_NONE)
		{
			if (identifier == "proc")
				return parse_procedure();
			if(identifier == "package")
				return parse_package_declaration();
			if(identifier == "import")
				return parse_package_import();
		}
		
		token_result = get_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			return expression{};
		}
		
		if (token_result.value()->type == ':')
		{
			return parse_variable_declaration(CONTEXT_STATEMENT, identifier, specifiers_result.value());
		}
		
		report_parse_error("Unexpected token", this->token_index - 1);
		recover();

		return expression{};
	}

	expression parser::parse_statement()
	{
		result<u8> specifiers_result = parse_specifiers();
		if(!specifiers_result)
			return expression{};
		
		// Next token is guaranteed to be an identifier
		
		result<token*> token_result = get_token();
		const std::string& identifier = input->identifiers[token_result.value()->value_index];
		
		// Return statement
		if(identifier == "return")
		{
			return parse_expression(CONTEXT_STATEMENT);
			
			// TODO: Check if type of returned value matches procedure return type
		}
		
		token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;
				
			return expression{};
		}
		
		if(specifiers_result.value() == SPECIFIER_NONE)
		{
			if(token_result.value()->type == '(')
			{
				// Eat the '('
				get_token();

				return parse_call(CONTEXT_STATEMENT, identifier);
			}
		}
		
		if(token_result.value()->type == ':')
		{
			// Eat the ':'
			get_token();
			
			return parse_variable_declaration(CONTEXT_STATEMENT, identifier,
				specifiers_result.value());
		}
		else
		{
			report_parse_error("Expected ':'", this->token_index - 1);
			recover();
			
			return expression{};
		}
	}

	expression parser::parse_expression(parse_context context)
	{
		expression primary = parse_primary(CONTEXT_NONE);

		result<token*> token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;
			
			return expression{};
		}

		result<const binary_operator*> op_result = get_op(token_result.value()->type);
		if (!op_result)
		{
			// Next token is not a binary operator

			if (context == CONTEXT_STATEMENT)
			{
				if (token_result.value()->type != ';')
				{
					report_parse_error("Expected ';'", this->token_index - 1);
					recover();
					
					return expression{};
				}

				// Eat the ';'
				get_token();
			}

			return primary;
		}

		// Eat the binary operator
		get_token();

		// Parse right-hand side of binary expression
		return parse_binary(context, primary, op_result.value());
	}
		
	expression parser::parse_primary(parse_context context)
	{
		result<token*> token_result = peek_token();
		if(!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;
			
			return expression{};
		}
		
		const std::string* value;
		switch(token_result.value()->type)
		{
			default:
				report_parse_error("Unexpected token", this->token_index - 1);
				recover();
				
				return expression{};
			
			case '&':
				expression_unary expr_address_of;
				expr_address_of.op_code = token_result.value()->type;
				
				// Eat the '&'
				get_token();
				
				// TODO: Delete this later
				expr_address_of.expr = new expression;
				*expr_address_of.expr = parse_primary(context);
				
				return expression{ expr_address_of };
			
			case '^':
				expression_unary expr_dereference;
				expr_dereference.op_code = token_result.value()->type;
				
				// Eat the '^'
				get_token();
				
				// TODO: Delete this later
				expr_dereference.expr = new expression;
				*expr_dereference.expr = parse_primary(context);
				
				return expression{ expr_dereference };
			
			case TOKEN_IDENTIFIER:
				value = &input->identifiers[token_result.value()->value_index];
				
				// Eat the identifier
				get_token();
				
				if (*value == "proc")
				{
					report_parse_error(
						"Procedure must be top-level expression",
						this->token_index - 1
					);
					
					recover(); // TODO: Jump to end of procedure and not next ';' or 'proc'
					return expression{};
				}

				// Look ahead after the identifier
				token_result = peek_token();
				if(!token_result)
				{
					report_parse_error("Expected a token", this->token_index - 1);
					this->done = true;
					
					return expression{};
				}
				
				switch(token_result.value()->type)
				{
					default:
						return parse_reference(context, *value);
						
					case '(':
						// Eat the '('
						get_token();
						return parse_call(context, *value);
				}
				
			case TOKEN_INTEGER:
				value = &input->integers[token_result.value()->value_index];
				
				// Eat the integer literal
				get_token();
				return parse_number_literal(context, *value, NUMBER_INTEGER);

			case TOKEN_DECIMAL:
				value = &input->decimals[token_result.value()->value_index];
				
				// Eat the decimal literal
				get_token();
				return parse_number_literal(context, *value, NUMBER_DECIMAL);
				
			case TOKEN_STRING:
				value = &input->strings[token_result.value()->value_index];
				
				// Eat the string literal
				get_token();
				return parse_string_literal(context, *value);
			
			case '(':
				// Eat the '('
				get_token();
				return parse_parentheses(context);
		}
		
		report_parse_error("Undefined token", this->token_index - 1);
		recover();
		
		return expression{};
	}

	expression parser::parse_binary(parse_context context,
		const expression& left, const binary_operator* op)
	{
		// TODO: Free this later
		expression_binary binary{
			new expression{ left }, new expression{ parse_expression(context) }, op
		};

		return expression{ binary };
	}
	
	expression parser::parse_parentheses(parse_context context)
	{
		// Parse what is inside the parentheses
		expression expr = parse_expression(CONTEXT_NONE);
		
		// Eat the ')'
		result<token*> token_result = get_token();
		if(!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			return expression{};
		}
		
		if(token_result.value()->type != ')')
		{
			report_parse_error("Expected ')'", this->token_index - 1);
			recover();
			
			return expression{};
		}
		
		if(context == CONTEXT_STATEMENT)
		{
			// Eat the ';'
			result<token*> token_result = get_token();
			if(!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
			
			if(token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				
				return expression{};
			}
		}
		
		// Return either the primary expression that was encased in parentheses or
		// an `expression_parentheses` if binary expressions were encased.
		if(expr.value.empty.type == EXPR_BINARY)
			return expression{ expression_parentheses{ expr.value.binary.value } };
		
		return expr;
	}
	
	expression parser::parse_number_literal(parse_context context,
		const std::string& number, number_type type)
	{
		if (context == CONTEXT_STATEMENT)
		{
			result<token*> token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				return expression{};
			}

			if (token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				
				return expression{};
			}

			// Eat the ';'
			get_token();
		}
		
		return expression{ expression_number_literal{ type, number } };
	}
	
	expression parser::parse_string_literal(parse_context context, const std::string& str)
	{
		if (context == CONTEXT_STATEMENT)
		{
			result<token*> token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				return expression{};
			}

			if (token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				
				return expression{};
			}

			// Eat the ';'
			get_token();
		}
		
		return expression{ expression_string_literal{ str } };
	}

	expression parser::parse_reference(parse_context context, const std::string& identifier)
	{
		scope* current_scope = global_scope.get_child(current_scope_index);
		result<symbol> reference_result = current_scope->find_symbol(identifier, global_scope);
		
		// Check if symbol is visible in current scope
		if (!reference_result)
		{
			report_parse_error(
				"Undefined symbol or symbol is not visible in current scope",
				this->token_index - 1
			);
			
			return expression{};
		}

		if (context == CONTEXT_STATEMENT)
		{
			result<token*> token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				return expression{};
			}

			if (token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				
				return expression{};
			}

			// Eat the ';'
			get_token();
		}

		return expression{ expression_reference{ reference_result.value() } };
	}
	
	expression parser::parse_variable_declaration(parse_context context,
		const std::string& variable_name, u8 specifiers)
	{
		bool is_pointer;
		
		result<token*> token_result = get_token();
		if(!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			return expression{};
		}
		
		if (token_result.value()->type == '^')
		{
			is_pointer = true;
			
			// Eat the '^'
			token_result = get_token();
			if(!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
		}
		else
		{
			is_pointer = false;
		}
		
		if (token_result.value()->type != TOKEN_IDENTIFIER)
		{
			report_parse_error("Expected variable type", this->token_index - 1);
			recover();
			
			return expression{};
		}

		// Parse the variable type
		const std::string& variable_type_name =
			input->identifiers[token_result.value()->value_index];
		
		// Check if the type is visible in current scope
		scope* current_scope = global_scope.get_child(current_scope_index);
		result<type> find_type_result = current_scope->find_type(
			variable_type_name, global_scope
		);
		
		if (!find_type_result)
		{
			report_parse_error(
				"Undefined type or type is not visible in current scope",
				this->token_index - 1
			);
			
			return expression{};
		}
		
		// Add variable to symbol table of current scope
		bool add_symbol_result = current_scope->add_symbol(
			symbol{ variable_name, *current_package_name(), SYMBOL_VARIABLE, variable_type_name }, global_scope
		);
		
		if (!add_symbol_result)
		{
			report_parse_error("Symbol is already defined", this->token_index - 1);
			return expression{};
		}

		if (context == CONTEXT_STATEMENT)
		{
			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				return expression{};
			}

			// Optional variable assignment
			if (token_result.value()->type == OP_EQUALS.op_code)
			{
				// Eat the '='
				get_token();

				// Parse the right-hand side
				return parse_binary(
					CONTEXT_STATEMENT,
					expression{
						expression_variable_declaration{
							symbol{ variable_name },
							find_type_result.value().name,
							specifiers,
							is_pointer
						} 
					},
					&OP_EQUALS
				);
			}

			// Expecting ';'
			if (token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				
				return expression{};
			}

			// Eat the ';'
			get_token();
			
			return expression{
				expression_variable_declaration{
					symbol{ variable_name },
					find_type_result.value().name,
					specifiers,
					is_pointer
				}
			};
		}

		return expression{
			expression_variable_declaration{
				symbol{ variable_name },
				find_type_result.value().name,
				specifiers,
				is_pointer
			}
		};
	}

	expression parser::parse_call(parse_context context, const std::string& procedure_name)
	{
		// Check if the procedure is visible in current scope
		scope* current_scope = global_scope.get_child(current_scope_index);
		result<symbol> proc_result = current_scope->find_symbol(procedure_name, global_scope);
		if (!proc_result)
		{
			report_parse_error(
				"Undefined symbol or symbol is not visible in current scope",
				this->token_index - 1
			);
			
			return expression{};
		}

		result<token*> token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;
			return expression{};
		}

		expression_procedure_call call_expr{
			symbol{ procedure_name, *current_package_name(), SYMBOL_PROCEDURE }, std::vector<expression>{}
		};

		if (token_result.value()->type != ')')
		{
			// Parse arguments
			while (true)
			{
				expression argument = parse_expression(CONTEXT_NONE);
				if (argument.value.empty.type == EXPR_EMPTY)
				{
					report_parse_error("Unexpected token", this->token_index - 1);
					recover();
					return expression{};
				}

				call_expr.argument_list.push_back(argument);

				token_result = peek_token();
				if (!token_result)
				{
					report_parse_error("Expected a token", this->token_index - 1);
					this->done = true;
					return expression{};
				}

				if (token_result.value()->type == ',')
				{
					// Eat the ',' and parse the next argument
					get_token();
					continue;
				}
				else if (token_result.value()->type == ')')
				{
					// Eat the ')'
					get_token();
					break;
				}
				else
				{
					report_parse_error("Expected ',' or ')'", this->token_index - 1);
					recover();
					return expression{};
				}
			}
		}
		else
		{
			// No arguments

			// Eat the ')'
			get_token();
		}

		if (context == CONTEXT_STATEMENT)
		{
			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				return expression{};
			}

			if (token_result.value()->type != ';')
			{
				report_parse_error("Expected ';'", this->token_index - 1);
				recover();
				return expression{};
			}

			// Eat the ';'
			get_token();
		}

		return expression{ call_expr };
	}

	std::vector<expression> parser::parse_argument_list()
	{
		std::vector<expression> argument_list;

		// Get the first token after '('
		// No need to check if it's valid because that was done before in 'parse_procedure()'
		//result<token*> token_result = get_token();

		while (true)
		{
			result<u8> specifiers_result = parse_specifiers();
			if (!specifiers_result)
				return std::vector<expression>{};

			// Next token is guaranteed to be an identifier

			result<token*> token_result = get_token();
			const std::string& identifier = input->identifiers[token_result.value()->value_index];

			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;

				return std::vector<expression>{};
			}

			if (token_result.value()->type != ':')
			{
				report_parse_error("Unexpected token", this->token_index - 1);
				// TODO: Recover from this by going to the next argument

				return std::vector<expression>{};
			}

			// Eat the ':'
			get_token();

			// Parse the argument
			argument_list.push_back(parse_variable_declaration(CONTEXT_NONE, identifier, specifiers_result.value()));

			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;

				return std::vector<expression>{};
			}

			if (token_result.value()->type == ',')
			{
				// Eat the ','
				get_token();

				// Parse next argument
				continue;
			}

			if (token_result.value()->type == ')')
			{
				// Eat the ')'
				get_token();

				// Done parsing argument list
				return argument_list;
			}

			report_parse_error("Unexpected token", this->token_index - 1);
			recover();

			return std::vector<expression>{};
		}
	}

	expression parser::parse_procedure()
	{
		result<token*> token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;

			return expression{};
		}

		if (token_result.value()->type != TOKEN_IDENTIFIER)
		{
			report_parse_error("Expected procedure name", this->token_index - 1);
			recover();

			return expression{};
		}

		const std::string& procedure_name = input->identifiers[token_result.value()->value_index];

		// Eat the identifier
		get_token();

		// Add procedure to symbol table of current scope
		scope* current_scope = global_scope.get_child(current_scope_index);
		bool add_symbol_result = current_scope->add_symbol(
			symbol{ procedure_name, *current_package_name(), SYMBOL_PROCEDURE }, global_scope
		);
		
		if (!add_symbol_result)
		{
			report_parse_error("Symbol is already defined", this->token_index - 1);
			return expression{};
		}

		token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;

			return expression{};
		}

		if (token_result.value()->type != '(')
		{
			report_parse_error("Expected '('", this->token_index - 1);
			recover();

			return expression{};
		}

		// Eat the '('
		get_token();

		token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;

			return expression{};
		}

		// If a return type is specified it will be stashed here
		result<type> return_type_result;

		// Any arguments are stashed here, otherwise it's empty
		std::vector<expression> argument_list;

		// Procedure has no arguments
		if (token_result.value()->type == ')')
		{
			// Just eat the ')'
			get_token();
		}
		// Procedure has arguments
		else
		{
			// Parse argument list
			argument_list = parse_argument_list();
		}
		
		token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;
			
			return expression{};
		}

		// Return type specified
		if (token_result.value()->type == TOKEN_RIGHT_POINTER)
		{
			// Eat the '->'
			get_token();

			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				
				return expression{};
			}

			if (token_result.value()->type != TOKEN_IDENTIFIER)
			{
				report_parse_error("Expected return type", this->token_index - 1);
				recover();

				return expression{};
			}
			
			const std::string& return_type_name =
				input->identifiers[token_result.value()->value_index];

			// Check if the type is visible in current scope
			return_type_result = current_scope->find_type(return_type_name, global_scope);
			if (!return_type_result)
			{
				report_parse_error(
					"Undefined type or type is not visible in current scope",
					this->token_index - 1
				);
				
				return expression{};
			}

			// Eat the return type
			get_token();

			// Peek the next token
			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;

				return expression{};
			}
		}
			
		// Procedure has a body
		if (token_result.value()->type == '{')
		{
			// Eat the '{'
			get_token();

			// Parse procedure body
			return parse_procedure_body(
				expression_procedure_prototype {
					symbol { procedure_name, *current_package_name(), SYMBOL_PROCEDURE },
					argument_list,
					return_type_result ? return_type_result.value().name : TYPE_VOID.name
				}
			);
		}

		// Procedure has no body and is a prototype
		if (token_result.value()->type == ';')
		{
			// Eat the ';'
			get_token();

			// Done parsing procedure prototype
			return expression{
				expression_procedure_prototype {
					symbol { procedure_name, *current_package_name(), SYMBOL_PROCEDURE },
					argument_list,
					return_type_result ? return_type_result.value().name : TYPE_VOID.name
				}
			};
		}

		if (return_type_result)
			report_parse_error("Expected '{' or ';'", this->token_index - 1);
		else
			report_parse_error("Expected '{', return type or ';'", this->token_index - 1);

		recover();

		return expression{};
	}

	expression parser::parse_procedure_body(const expression_procedure_prototype& prototype)
	{
		result<token*> token_result = peek_token();
		if (!token_result)
		{
			report_parse_error("Expected a token", this->token_index - 1);
			this->done = true;

			return expression{};
		}

		if (token_result.value()->type == '}')
		{
			// Eat the peeked '}'
			get_token();

			// Procedure's body is empty
			return expression{
				expression_procedure_definition{ prototype, std::vector<expression>{} }
			};
		}

		// Create new scope (only if procedure body is not empty)
		scope_index parent_scope = current_scope_index;
		scope* current_scope = global_scope.get_child(current_scope_index);
		current_scope_index = current_scope->add_child(scope{ prototype.name.name });

		std::vector<expression> body;

		while (true)
		{
			body.push_back(parse_statement());

			token_result = peek_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				this->done = true;
				
				current_scope_index = parent_scope;
				return expression{};
			}

			// Check if the token after the last parsed expression is '}',
			// which would be the end of the body.
			if (token_result.value()->type == '}')
			{
				// Eat the '}'
				get_token();
				break;
			}
		}

		current_scope_index = parent_scope;
		return expression{ expression_procedure_definition{ prototype, body } };
	}
	
	expression parser::parse_package_declaration()
	{
		result<token*> token_result;
		std::string package_name;
		
		while(true)
		{
			token_result = get_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
				
			if(token_result.value()->type != TOKEN_IDENTIFIER)
			{
				report_parse_error("Expected package name", this->token_index - 1);
				return expression{};
			}
			
			package_name += input->identifiers[token_result.value()->value_index];
			
			token_result = get_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
			
			if(token_result.value()->type == '.')
			{
				package_name += ".";
				continue;
			}
			else if(token_result.value()->type == ';')
			{
				// Done parsing package declaration statement
				set_current_package(package_name);
				return expression{ expression_package_declaration{ package_name } };
			}
			else
			{
				report_parse_error("Unexpected token", this->token_index - 1);
				recover();
				return expression{};
			}
		}
	}
	
	expression parser::parse_package_import()
	{
		result<token*> token_result;
		std::string package_name;
		
		while(true)
		{
			token_result = get_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
				
			if(token_result.value()->type != TOKEN_IDENTIFIER)
			{
				report_parse_error("Expected package name", this->token_index - 1);
				return expression{};
			}
			
			package_name += input->identifiers[token_result.value()->value_index];
			
			token_result = get_token();
			if (!token_result)
			{
				report_parse_error("Expected a token", this->token_index - 1);
				return expression{};
			}
			
			// TODO: Check for "as" token
			if(token_result.value()->type == '.')
			{
				package_name += ".";
				continue;
			}
			else if(token_result.value()->type == ';')
			{
				// Done parsing package declaration statement
				return expression{ expression_package_import{ package_name } };
			}
			else
			{
				report_parse_error("Unexpected token", this->token_index - 1);
				recover();
				return expression{};
			}
		}
	}
	
	std::string parser::format_expression(const expression& expr, u64 level)
	{
		std::string message;
		for (u64 i = 0; i < level; i += 1)
		{
			message += "    ";
		}

		// This is safe because no matter the current variant,
		// it is guaranteed to be at the same address
		// (and type is the first member in tagged expression struct).
		switch (expr.value.empty.type)
		{
		default:
			message += "Unknown Expression";
			break;
		case EXPR_EMPTY:
			message += "Empty Expression";
			break;
		case EXPR_BINARY:
			message += "Binary Expression: Op_Code='"
				+ std::to_string(expr.value.binary.value.op->op_code) + "'"
				+ "\n" + format_expression(*expr.value.binary.value.left, level + 1)
				+ "\n" + format_expression(*expr.value.binary.value.right, level + 1);
			break;
		case EXPR_PARENTHESES:
			message += "Parentheses Expression: Op_Code='"
				+ std::to_string(expr.value.parentheses.value.expr.op->op_code) + "'"
				+ "\n" + format_expression(*expr.value.parentheses.value.expr.left, level + 1)
				+ "\n" + format_expression(*expr.value.parentheses.value.expr.right, level + 1);
			break;
		case EXPR_NUMBER_LITERAL:
			message += "Number Literal: Value='"
				+ expr.value.number.value.value
				+ "'";
			break;
		case EXPR_STRING_LITERAL:
			message += "String Literal: Value='"
				+ expr.value.str.value.value
				+ "'";
			break;
		case EXPR_REFERENCE:
			message += "Reference: Name='"
				+ expr.value.reference.value.name.name
				+ "'";
			break;
		case EXPR_VAR_DECLARATION:
			message += "Variable Declaration: Name='"
				+ expr.value.variable_declaration.value.name.name
				+ "' Type='";
			
			if(expr.value.variable_declaration.value.is_pointer)
				message += "^";
			
			message += expr.value.variable_declaration.value.type_name
				+ "'";
			
			if(expr.value.variable_declaration.value.specifiers != SPECIFIER_NONE)
			{
				message += " Specifiers='";
				
				if(expr.value.variable_declaration.value.specifiers & SPECIFIER_CONST)
					message += "const ";
				
				if(expr.value.variable_declaration.value.specifiers & SPECIFIER_MUT)
					message += "mut ";
				
				message += "'";
			}
			
			break;
		case EXPR_PROC_PROTOTYPE:
			message += "Procedure Prototype Expression: Name='"
				+ expr.value.procedure_prototype.value.name.name
				+ "' Return Type='" + expr.value.procedure_prototype.value.return_type_name
				+ "'";

			if (expr.value.procedure_prototype.value.argument_list.size() > 0)
				message += "\nArgument List: ";

			for (u64 i = 0; i < expr.value.procedure_prototype.value.argument_list.size(); i += 1)
			{
				message += "\n" + format_expression(
					expr.value.procedure_prototype.value.argument_list[i], level + 1
				);
			}
			break;
		case EXPR_PROC_DEFINITION:
			message += format_expression(
				expression{ expr.value.procedure_definition.value.prototype }, level
			);

			if (expr.value.procedure_definition.value.body.size() > 0)
				message += "\nBody: ";

			for (u64 i = 0; i < expr.value.procedure_definition.value.body.size(); i += 1)
			{
				message += "\n" + format_expression(
					expr.value.procedure_definition.value.body[i], level + 1);
			}
			break;
		case EXPR_PROC_CALL:
			message += "Procedure Call Expression: Name='"
				+ expr.value.procedure_call.value.name.name
				+ "'";

			for (u64 i = 0; i < expr.value.procedure_call.value.argument_list.size(); i += 1)
			{
				message += "\n" + format_expression(
					expr.value.procedure_call.value.argument_list[i], level + 1
				);
			}
			break;
		case EXPR_PACKAGE_DECLARATION:
			message += "Package Declaration: Name='" +
				expr.value.package_declaration.value.package_name +
				"'";
			break;
		case EXPR_PACKAGE_IMPORT:
			message += "Package Import: Name='" +
				expr.value.package_import.value.package_name +
				"'";
			break;
		}

		return message;
	}
	
	void parser::print_expressions()
	{
		for (u64 i = 0; i < output->expressions.size(); i += 1)
		{
			std::cout << format_expression(output->expressions[i]) << std::endl;
		}
	}
	
	expression_binary* get_binary_expression(expression* expr)
	{
		if(expr->value.empty.type == EXPR_BINARY)
    		return &expr->value.binary.value;
    	else if(expr->value.empty.type == EXPR_PARENTHESES)
    		return &expr->value.parentheses.value.expr;
    	
    	return nullptr;
	}
}