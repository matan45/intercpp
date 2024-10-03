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

#include "Lexer.hpp"

// Define the type of function signature used for callable functions
using ScriptFunction = std::function<double(const std::vector<double>&)>;

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
	Environment() {}

	// Register a built-in C++ function by name
	void registerFunction(const std::string& name, ScriptFunction func) {
		if (functionRegistry.find(name) != functionRegistry.end()) {
			throw std::runtime_error("Function already registered: " + name);
		}
		functionRegistry[name] = func;
	}

	// Register a user-defined function (AST-based)
	void registerUserFunction(const std::string& name, ASTNode* functionNode) {
		if (userFunctionRegistry.find(name) != userFunctionRegistry.end()) {
			throw std::runtime_error("User-defined function already registered: " + name);
		}
		userFunctionRegistry[name] = functionNode;
	}

	// Evaluate a function by name with given arguments
	double evaluateFunction(const std::string& name, const std::vector<double>& args) const {
		// Check if the function is a C++ native function
		if (functionRegistry.find(name) != functionRegistry.end()) {
			return functionRegistry.at(name)(args);
		}

		// Check if the function is a user-defined function
		if (userFunctionRegistry.find(name) != userFunctionRegistry.end()) {
			// Assuming ASTNode supports a context to manage arguments for user functions
			ASTNode* functionNode = userFunctionRegistry.at(name);
			return functionNode->evaluate(*const_cast<Environment*>(this));
		}

		throw std::runtime_error("Undefined function: " + name);
	}

	// Declare a variable by name and type
	void declareVariable(const std::string& name, ValueType type) {
		if (variableTable.find(name) != variableTable.end()) {
			throw std::runtime_error("Variable already declared: " + name);
		}
		variableTable[name] = { 0, type };  // Initialize with default value 0
	}

	// Set a variable's value
	void setVariable(const std::string& name, double value) {
		if (variableTable.find(name) == variableTable.end()) {
			throw std::runtime_error("Undefined variable: " + name);
		}

		ValueType type = variableTable[name].second;

		// Perform type checks to ensure correctness
		if (type == ValueType::INT) {
			if (value != static_cast<int>(value)) {
				throw std::runtime_error("Type error: Expected int value for variable " + name);
			}
		}

		// Assign the value to the variable
		variableTable[name].first = value;
	}

	// Get a variable's value
	double getVariable(const std::string& name) const {
		if (variableTable.find(name) == variableTable.end()) {
			throw std::runtime_error("Undefined variable: " + name);
		}
		return variableTable.at(name).first;
	}

private:
	// Registry for native C++ functions
	std::unordered_map<std::string, ScriptFunction> functionRegistry;

	// Registry for user-defined functions
	std::unordered_map<std::string, ASTNode*> userFunctionRegistry;

	// Table for managing variables (name -> (value, type))
	std::unordered_map<std::string, std::pair<double, ValueType>> variableTable;
};



// Program Node: Represents a collection of statements
struct ProgramNode : public ASTNode {
	std::vector<ASTNode*> statements;

	ProgramNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	double evaluate(Environment& env) const override {
		for (ASTNode* statement : statements) {
			statement->evaluate(env);
		}
		return 0; // Program as a whole doesn’t return a specific value
	}

	~ProgramNode() {
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

// Declaration Node: Represents a variable declaration, possibly with an initializer
struct DeclarationNode : public ASTNode {
	std::string variableName;
	ValueType type;
	ASTNode* initializer;

	DeclarationNode(const std::string& variableName, ValueType type, ASTNode* initializer = nullptr)
		: variableName(variableName), type(type), initializer(initializer) {}

	double evaluate(Environment& env) const override {
		env.declareVariable(variableName, type);
		if (initializer) {
			double value = initializer->evaluate(env);
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
		env.setVariable(variableName, value);
		return value;
	}

	~AssignmentNode() {
		delete expression;
	}
};

// Variable Node: Represents a reference to a variable
struct VariableNode : public ASTNode {
	std::string name;

	VariableNode(const std::string& name) : name(name) {}

	double evaluate(Environment& env) const override {
		return env.getVariable(name);
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
		std::vector<double> argValues;
		for (ASTNode* arg : arguments) {
			argValues.push_back(arg->evaluate(env));
		}
		return env.evaluateFunction(name, argValues);
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

