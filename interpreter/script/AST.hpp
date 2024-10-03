#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <utility>     // For std::make_pair

#include "Lexer.hpp"
#include <variant>

// Define the type of function signature used for callable functions
using ScriptFunction = std::function<double(const std::vector<double>&)>;
using VariableValue = std::variant<double, bool, std::string>;


// Define supported value types
enum class ValueType {
	INT,
	FLOAT,
	BOOL,
	STRING
};

class Environment;
// Base class for all AST nodes
struct ASTNode {
	virtual ~ASTNode() = default;
	virtual double evaluate(Environment& env) const = 0;
};

// The Environment class to manage functions and variables
class Environment {
public:
	Environment() = default;

	// Register a built-in C++ function by name
	void registerFunction(const std::string& name, ScriptFunction func) {
		if (functionRegistry.contains(name)) {
			throw std::runtime_error("Function already registered: " + name);
		}
		functionRegistry[name] = func;
	}

	// Register a user-defined function (AST-based)
	void registerUserFunction(const std::string& name, ASTNode* functionNode) {
		if (userFunctionRegistry.contains(name)) {
			throw std::runtime_error("User-defined function already registered: " + name);
		}
		userFunctionRegistry[name] = functionNode;
	}

	// Evaluate a function by name with given arguments
	double evaluateFunction(const std::string& name, const std::vector<double>& args) const {
		if (functionRegistry.contains(name)) {
			return functionRegistry.at(name)(args);
		}

		if (userFunctionRegistry.contains(name)) {
			ASTNode const* functionNode = userFunctionRegistry.at(name);
			return functionNode->evaluate(*const_cast<Environment*>(this));
		}

		throw std::runtime_error("Undefined function: " + name);
	}

	// Declare a variable by name and type
	void declareVariable(const std::string& name, ValueType type) {
		if (variableTable.contains(name)) {
			throw std::runtime_error("Variable already declared: " + name);
		}
		switch (type) {
		case ValueType::INT:
		case ValueType::FLOAT:
			// Initialize numeric types to 0
			variableTable[name] = std::make_pair(0.0, type);
			break;
		case ValueType::BOOL:
			// Initialize boolean to false
			variableTable[name] = std::make_pair(false, type);
			break;
		case ValueType::STRING:
			// Initialize string to an empty string
			variableTable[name] = std::make_pair(std::string(""), type);
			break;
		}
	}

	void setVariable(const std::string& name, const VariableValue& value) {
		if (!variableTable.contains(name)) {
			throw std::runtime_error("Undefined variable: " + name);
		}

		ValueType type = variableTable[name].second;

		// Perform type checks to ensure correctness
		if (type == ValueType::INT) {
			if (!std::holds_alternative<double>(value) ||
				std::get<double>(value) != static_cast<int>(std::get<double>(value))) {
				throw std::runtime_error("Type error: Expected int value for variable " + name);
			}
		}
		else if (type == ValueType::FLOAT) {
			if (!std::holds_alternative<double>(value)) {
				throw std::runtime_error("Type error: Expected float value for variable " + name);
			}
		}
		else if (type == ValueType::BOOL) {
			if (!std::holds_alternative<bool>(value)) {
				throw std::runtime_error("Type error: Expected boolean value for variable " + name);
			}
		}
		else if (type == ValueType::STRING) {
			if (!std::holds_alternative<std::string>(value)) {
				throw std::runtime_error("Type error: Expected string value for variable " + name);
			}
		}

		// Assign the value to the variable
		variableTable[name].first = value;
		std::cout << "Environment: Updated variable " << name << " to new value." << std::endl;
	}



	// Get a variable's value
	VariableValue getVariable(const std::string& name) const {
		if (!variableTable.contains(name)) {
			throw std::runtime_error("Undefined variable: " + name);
		}
		return variableTable.at(name).first;
	}

private:
	std::unordered_map<std::string, ScriptFunction> functionRegistry;
	std::unordered_map<std::string, ASTNode*> userFunctionRegistry;
	std::unordered_map<std::string, std::pair<VariableValue, ValueType>> variableTable;
};



// Program Node: Represents a collection of statements
struct ProgramNode : public ASTNode {
	std::vector<ASTNode*> statements;

	ProgramNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	double evaluate(Environment& env) const override {
		for (ASTNode const* statement : statements) {
			statement->evaluate(env);
		}
		return 0; // Program as a whole doesn’t return a specific value
	}

	~ProgramNode() override {
		for (ASTNode* statement : statements) {
			delete statement;
		}
	}
};

struct ForNode : public ASTNode {
	ASTNode* initializer;  // Variable declaration or assignment
	ASTNode* condition;    // Loop condition
	ASTNode* update;       // Update expression
	ASTNode* body;         // Loop body

	ForNode(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body)
		: initializer(initializer), condition(condition), update(update), body(body) {}

	double evaluate(Environment& env) const override {
		// Evaluate the initializer
		if (initializer) {
			initializer->evaluate(env);
		}

		// Loop while the condition is true
		double result = 0;
		while (condition->evaluate(env) != 0) {
			result = body->evaluate(env);
			if (update) {
				update->evaluate(env);
			}
		}
		return result;
	}

	~ForNode() {
		delete initializer;
		delete condition;
		delete update;
		delete body;
	}
};

struct BooleanNode : public ASTNode {
	bool value;

	BooleanNode(bool value) : value(value) {}

	double evaluate(Environment& env) const override {
		// Return 1.0 for true, 0.0 for false (to allow compatibility in numeric contexts)
		return value ? 1.0 : 0.0;
	}

	bool evaluateBool(Environment& env) const {
		return value;
	}
};


struct StringNode : public ASTNode {
	std::string value;

	StringNode(const std::string& value) : value(value) {}

	double evaluate(Environment& env) const override {
		// This method returns 0 as this is primarily for printing purposes.
		return 0;
	}

	std::string evaluateString(Environment& env) const {
		return value;
	}
};


struct DoWhileNode : public ASTNode {
	ASTNode* body;        // Loop body
	ASTNode* condition;   // Loop condition

	DoWhileNode(ASTNode* body, ASTNode* condition)
		: body(body), condition(condition) {}

	double evaluate(Environment& env) const override {
		double result = 0;
		do {
			result = body->evaluate(env);
		} while (condition->evaluate(env) != 0);
		return result;
	}

	~DoWhileNode() {
		delete body;
		delete condition;
	}
};


// Block Node: Represents a block of statements, typically for control flow or functions
struct BlockNode : public ASTNode {
	std::vector<ASTNode*> statements;

	BlockNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	double evaluate(Environment& env) const override {
		for (ASTNode* statement : statements) {
			statement->evaluate(env);
		}
		return 0; // A block doesn’t return a value directly
	}

	~BlockNode() {
		for (ASTNode* statement : statements) {
			delete statement;
		}
	}
};

// Variable Node: Represents a reference to a variable
struct VariableNode : public ASTNode {
	std::string name;

	VariableNode(const std::string& name) : name(name) {}

	double evaluate(Environment& env) const override {
		auto value = env.getVariable(name);
		if (std::holds_alternative<double>(value)) {
			double numValue = std::get<double>(value);
			std::cout << "VariableNode: Retrieved numeric value " << numValue << " for variable " << name << std::endl;
			return numValue;
		}
		else if (std::holds_alternative<bool>(value)) {
			bool boolValue = std::get<bool>(value);
			std::cout << "VariableNode: Retrieved boolean value " << (boolValue ? "true" : "false") << " for variable " << name << std::endl;
			return boolValue ? 1.0 : 0.0;
		}
		// Handle string type: return a default value or throw an error
		else if (std::holds_alternative<std::string>(value)) {
			std::string strValue = std::get<std::string>(value);
			std::cout << "VariableNode: Retrieved string value \"" << strValue << "\" for variable " << name << std::endl;
			// Option 1: Return a default value such as 0.0
			return 0.0;

			// Option 2: Throw an error if evaluating a string as a double is not allowed
			// throw std::runtime_error("Cannot evaluate a string value as a double for variable: " + name);
		}
		else {
			throw std::runtime_error("Expected numeric value for variable: " + name);
		}
	}

};

// Declaration Node: Represents a variable declaration, possibly with an initializer
struct DeclarationNode : public ASTNode {
	std::string variableName;
	ValueType type;
	ASTNode* initializer;

	DeclarationNode(const std::string& variableName, ValueType type, ASTNode* initializer = nullptr)
		: type(type), initializer(initializer), variableName(variableName) {}

	double evaluate(Environment& env) const override {
		// Declare the variable in the environment
		env.declareVariable(variableName, type);

		// If there is an initializer, evaluate it and set the value in the environment
		if (initializer) {
			VariableValue value;

			switch (type) {
			case ValueType::INT:
			case ValueType::FLOAT: {
				double evaluatedValue = initializer->evaluate(env);
				value = evaluatedValue;
				std::cout << "DeclarationNode: Initializing variable " << variableName << " with value " << evaluatedValue << std::endl;
				break;
			}
			case ValueType::BOOL: {
				bool evaluatedValue = initializer->evaluate(env) != 0;
				value = evaluatedValue;
				std::cout << "DeclarationNode: Initializing boolean variable " << variableName << " with value " << (evaluatedValue ? "true" : "false") << std::endl;
				break;
			}
			case ValueType::STRING: {
				// Assuming the initializer can be evaluated to a string
				auto strNode = dynamic_cast<StringNode*>(initializer);
				if (strNode) {
					value = strNode->evaluateString(env);
					std::cout << "DeclarationNode: Initializing string variable " << variableName << " with value \"" << std::get<std::string>(value) << "\"" << std::endl;
				}
				else {
					throw std::runtime_error("Type error: Expected string initializer for variable " + variableName);
				}
				break;
			}
			default:
				throw std::runtime_error("Unknown value type for variable " + variableName);
			}

			// Set the value in the environment
			env.setVariable(variableName, value);
		}
		return 0;
	}

	~DeclarationNode() {
		delete initializer;
	}
};

// Assignment Node: Represents assigning a value to a variable
struct AssignmentNode : public ASTNode {
	std::string variableName;
	ASTNode* expression;

	AssignmentNode(const std::string& variableName, ASTNode* expression)
		: variableName(variableName), expression(expression) {}

	double evaluate(Environment& env) const override {
		double value = expression->evaluate(env);
		std::cout << "AssignmentNode: Assigning value " << value << " to variable " << variableName << std::endl;
		env.setVariable(variableName, value);
		return value;
	}

	~AssignmentNode() {
		delete expression;
	}
};


// Number Node: Represents a numerical literal
struct NumberNode : public ASTNode {
	double value;

	NumberNode(double value) : value(value) {}

	double evaluate(Environment& env) const override {
		return value;
	}
};

// Function Definition Node: Represents a user-defined function
struct FunctionNode : public ASTNode {
	std::string name;
	ValueType returnType;
	std::vector<std::pair<std::string, ValueType>> parameters;
	ASTNode* body;

	FunctionNode(const std::string& name, ValueType returnType,
		const std::vector<std::pair<std::string, ValueType>>& parameters, ASTNode* body)
		: name(name), returnType(returnType), parameters(parameters), body(body) {}

	double evaluate(Environment& env) const override {
		// FunctionNode itself is not "evaluated"; it's stored in the environment
		throw std::runtime_error("FunctionNode cannot be directly evaluated.");
	}

	~FunctionNode() {
		delete body;
	}
};

// Function Call Node: Represents calling a function with arguments
struct FunctionCallNode : public ASTNode {
	std::string name;
	std::vector<ASTNode*> arguments;

	FunctionCallNode(const std::string& name, const std::vector<ASTNode*>& arguments)
		: name(name), arguments(arguments) {}

	double evaluate(Environment& env) const override {
		if (name == "print") {
			if (arguments.size() != 1) {
				throw std::runtime_error("print expects 1 argument");
			}

			ASTNode* arg = arguments[0];

			// Handle variable node
			if (auto varNode = dynamic_cast<VariableNode*>(arg)) {
				auto value = env.getVariable(varNode->name);
				if (std::holds_alternative<double>(value)) {
					std::cout << "Print from script: " << std::get<double>(value) << std::endl;
				}
				else if (std::holds_alternative<std::string>(value)) {
					std::cout << "Print from script: " << std::get<std::string>(value) << std::endl;
				}
				else if (std::holds_alternative<bool>(value)) {
					std::cout << "Print from script: " << (std::get<bool>(value) ? "true" : "false") << std::endl;
				}
				else {
					throw std::runtime_error("Unsupported type for print function");
				}
			}
			// Handle direct number or string nodes
			else if (auto numNode = dynamic_cast<NumberNode*>(arg)) {
				std::cout << "Print from script: " << numNode->value << std::endl;
			}
			else if (auto strNode = dynamic_cast<StringNode*>(arg)) {
				std::cout << "Print from script: " << strNode->value << std::endl;
			}
			else {
				throw std::runtime_error("Unsupported type for print function");
			}
		}
		else {
			throw std::runtime_error("Undefined function: " + name);
		}

		return 0; // Function calls like print do not return values
	}

	~FunctionCallNode() {
		for (ASTNode* arg : arguments) {
			delete arg;
		}
	}
};

// Return Node: Represents a return statement in a function
struct ReturnNode : public ASTNode {
	ASTNode* returnValue;

	ReturnNode(ASTNode* returnValue) : returnValue(returnValue) {}

	double evaluate(Environment& env) const override {
		return returnValue->evaluate(env);
	}

	~ReturnNode() {
		delete returnValue;
	}
};

// If Node: Represents an if-else statement
struct IfNode : public ASTNode {
	ASTNode* condition;
	ASTNode* thenBranch;
	ASTNode* elseBranch;

	IfNode(ASTNode* condition, ASTNode* thenBranch, ASTNode* elseBranch = nullptr)
		: condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

	double evaluate(Environment& env) const override {
		double conditionValue = condition->evaluate(env);
		std::cout << "IfNode: conditionValue" << conditionValue << std::endl;
		if (conditionValue != 0) {
			return thenBranch->evaluate(env);
		}
		else if (elseBranch) {
			return elseBranch->evaluate(env);
		}
		return 0;
	}

	~IfNode() {
		delete condition;
		delete thenBranch;
		delete elseBranch;
	}
};

// While Node: Represents a while loop
struct WhileNode : public ASTNode {
	ASTNode* condition;
	ASTNode* body;

	WhileNode(ASTNode* condition, ASTNode* body) : condition(condition), body(body) {}

	double evaluate(Environment& env) const override {
		double result = 0;
		while (condition->evaluate(env) != 0) {
			result = body->evaluate(env);
		}
		return result;
	}

	~WhileNode() {
		delete condition;
		delete body;
	}
};

// Binary Operation Node: Represents a binary operation like +, -, *, /
struct BinaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* left;
	ASTNode* right;

	BinaryOpNode(TokenType op, ASTNode* left, ASTNode* right)
		: op(op), left(left), right(right) {}

	double evaluate(Environment& env) const override {
		double leftValue = left->evaluate(env);
		double rightValue = right->evaluate(env);

		switch (op) {
		case TokenType::PLUS: return leftValue + rightValue;
		case TokenType::MINUS: return leftValue - rightValue;
		case TokenType::MULTIPLY: return leftValue * rightValue;
		case TokenType::DIVIDE:
			if (rightValue == 0) {
				throw std::runtime_error("Division by zero");
			}
			return leftValue / rightValue;
		case TokenType::AND: return (leftValue != 0 && rightValue != 0) ? 1 : 0;
		case TokenType::OR: return (leftValue != 0 || rightValue != 0) ? 1 : 0;
		case TokenType::EQUALS: return (leftValue == rightValue) ? 1 : 0;
		case TokenType::NOT_EQUALS: return (leftValue != rightValue) ? 1 : 0;
		case TokenType::LESS: return (leftValue < rightValue) ? 1 : 0;
		case TokenType::LESS_EQUALS: return (leftValue <= rightValue) ? 1 : 0;
		case TokenType::GREATER: return (leftValue > rightValue) ? 1 : 0;
		case TokenType::GREATER_EQUALS: return (leftValue >= rightValue) ? 1 : 0;
		default: throw std::runtime_error("Unknown binary operator");
		}
	}

	~BinaryOpNode() {
		delete left;
		delete right;
	}
};

// Unary Operation Node: Represents a unary operation like - or !
struct UnaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* operand;

	UnaryOpNode(TokenType op, ASTNode* operand) : op(op), operand(operand) {}

	double evaluate(Environment& env) const override {
		double value = operand->evaluate(env);
		switch (op) {
		case TokenType::MINUS: return -value;
		case TokenType::NOT: return (value == 0) ? 1 : 0;
		default: throw std::runtime_error("Unknown unary operator");
		}
	}

	~UnaryOpNode() {
		delete operand;
	}
};

