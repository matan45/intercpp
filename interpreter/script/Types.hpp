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
	IMPORT,
	RBRACKET,
	LBRACKET,
	CLASS,         
	NEW,            
	DOT,           
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
	MAP,
	CLASS 
};

// Forward declare the struct
struct VariableValue;
class  Environment;
// Forward declaration of FunctionNode
struct FunctionNode;

// Define an alias to the variant that includes possible value types
using ValueVariant = std::variant<
	double,                                  // INT, FLOAT (handled as double for simplicity)
	bool,                                    // BOOL
	std::string,                             // STRING
	std::vector<VariableValue>,              // ARRAY: vector of VariableValue elements
	std::unordered_map<std::string, VariableValue> ,// MAP: unordered map with string keys
	FunctionNode*  // Function pointer type (for member functions)
>;

struct VariableValue {
	ValueVariant value;

	std::string toString() const {
		if (auto doublePtr = std::get_if<double>(&value)) {
			return std::to_string(*doublePtr);
		}
		else if (auto boolPtr = std::get_if<bool>(&value)) {
			return *boolPtr ? "true" : "false";
		}
		else if (auto strPtr = std::get_if<std::string>(&value)) {
			return *strPtr;
		}
		else if (auto arrayPtr = std::get_if<std::vector<VariableValue>>(&value)) {
			std::string result = "[";
			for (size_t i = 0; i < arrayPtr->size(); ++i) {
				if (i > 0) result += ", ";
				result += (*arrayPtr)[i].toString();
			}
			result += "]";
			return result;
		}
		else if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&value)) {
			std::string result = "{";
			bool first = true;
			for (const auto& [key, value] : *mapPtr) {
				if (!first) result += ", ";
				first = false;
				result += key + ": " + value.toString();
			}
			result += "}";
			return result;
		}
		return "unknown type";
	}

};


using ScriptFunction = std::function<VariableValue(const std::vector<VariableValue>&, const std::vector<std::string>&, Environment&)>;
