#pragma once
#include <variant>
#include <functional>
#include <string>

enum class TokenType {
	IDENTIFIER,
	NUMBER,
	STRING_LITERAL,
	PLUS, MINUS, MULTIPLY, DIVIDE,
	ASSIGN, // =
	LPAREN, RPAREN, // ( )
	LBRACE, RBRACE, // { }
	COMMA, SEMICOLON, // , ;
	AND, OR, NOT, // &&, ||, !
	EQUALS, NOT_EQUALS, // ==, !=
	LESS, LESS_EQUALS, GREATER, GREATER_EQUALS, // <, <=, >, >=
	FUNC, RETURN, IF, ELSE, WHILE, FOR, // Keywords
	INT, FLOAT, BOOL, STRING_TYPE, // Data types
	TRUE, FALSE, DO,
	END // End of input
};

// Define supported value types
enum class ValueType {
	INT,
	FLOAT,
	BOOL,
	STRING
};

// Define the type of function signature used for callable functions
using ScriptFunction = std::function<double(const std::vector<double>&)>;
using VariableValue = std::variant<double, bool, std::string>;