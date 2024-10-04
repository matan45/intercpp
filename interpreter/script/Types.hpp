#pragma once
#include <variant>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

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
	INT, FLOAT, BOOL, VOID_TYPE, STRING_TYPE, // Data types
	TRUE, FALSE, DO,
	PLUSPLUS, MINUSMINUS,
	ARRAY,
	MAP,
	RBRACKET,
	LBRACKET,
	COLON,
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
	VOID_TYPE,
	STRING,
	ARRAY,
	MAP
};

// Forward declare the struct
struct VariableValue;

// Define an alias to the variant that includes possible value types
using ValueVariant = std::variant<
	double,                                  // INT, FLOAT (handled as double for simplicity)
	bool,                                    // BOOL
	std::string,                             // STRING
	std::vector<VariableValue>,              // ARRAY: vector of VariableValue elements
	std::unordered_map<std::string, VariableValue>  // MAP: unordered map with string keys
>;

struct VariableValue {
	ValueVariant value;
};


using ScriptFunction = std::function<VariableValue(const std::vector<VariableValue>&)>;