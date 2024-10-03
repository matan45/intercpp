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
	PLUSPLUS, MINUSMINUS,
	END // End of input
};

enum class IncrementType {
	PREFIX,
	POSTFIX
};

// Define supported value types
enum class ValueType {
	INT,
	FLOAT,
	BOOL,
	STRING
};

// Define the type of function signature used for callable functions
using VariableValue = std::variant<double, bool, std::string>;
using ScriptFunction = std::function<VariableValue(const std::vector<VariableValue>&)>;