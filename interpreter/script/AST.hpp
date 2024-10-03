#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>

#include "Types.hpp"

class Environment;

// Base class for all AST nodes
struct ASTNode {
	virtual ~ASTNode() = default;
	virtual VariableValue evaluate(Environment& env) const = 0;//todo change the return type to VariableValue
};

// Program Node: Represents a collection of statements
struct ProgramNode : public ASTNode {
	std::vector<ASTNode*> statements;

	explicit ProgramNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	VariableValue evaluate(Environment& env) const override;

	~ProgramNode() override;
};

struct IncrementNode : public ASTNode {
	IncrementType incrementType;
	std::string variableName;

	explicit IncrementNode(IncrementType type, const std::string& varName)
		: incrementType(type), variableName(varName) {}

	VariableValue evaluate(Environment& env) const override;
	~IncrementNode() override = default;

};

struct ForNode : public ASTNode {
	ASTNode* initializer;  // Variable declaration or assignment
	ASTNode* condition;    // Loop condition
	ASTNode* update;       // Update expression
	ASTNode* body;         // Loop body

	explicit ForNode(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body)
		: initializer(initializer), condition(condition), update(update), body(body) {}

	VariableValue evaluate(Environment& env) const override;

	~ForNode() override;
};

struct BooleanNode : public ASTNode {
	bool value;

	explicit BooleanNode(bool value) : value(value) {}

	VariableValue evaluate(Environment& env) const override;

	~BooleanNode() override = default;
};


struct StringNode : public ASTNode {
	std::string value;

	explicit StringNode(const std::string& value) : value(value) {}

	VariableValue evaluate(Environment& env) const override;

	~StringNode() override = default;
};


struct DoWhileNode : public ASTNode {
	ASTNode* body;        // Loop body
	ASTNode* condition;   // Loop condition

	explicit DoWhileNode(ASTNode* body, ASTNode* condition)
		: body(body), condition(condition) {}

	VariableValue evaluate(Environment& env) const override;

	~DoWhileNode() override;
};


// Block Node: Represents a block of statements, typically for control flow or functions
struct BlockNode : public ASTNode {
	std::vector<ASTNode*> statements;

	explicit BlockNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	VariableValue evaluate(Environment& env) const override;

	~BlockNode() override;
};

// Variable Node: Represents a reference to a variable
struct VariableNode : public ASTNode {
	std::string name;

	explicit VariableNode(const std::string& name) : name(name) {}

	VariableValue evaluate(Environment& env) const override;

	~VariableNode() override = default;
};

// Declaration Node: Represents a variable declaration, possibly with an initializer
struct DeclarationNode : public ASTNode {
	std::string variableName;
	ValueType type;
	ASTNode* initializer;

	explicit DeclarationNode(const std::string& variableName, ValueType type, ASTNode* initializer = nullptr)
		: variableName(variableName), type(type), initializer(initializer) {}

	VariableValue evaluate(Environment& env) const override;

	~DeclarationNode() override;
};

// Assignment Node: Represents assigning a value to a variable
struct AssignmentNode : public ASTNode {
	std::string variableName;
	ASTNode* expression;

	explicit AssignmentNode(const std::string& variableName, ASTNode* expression)
		: variableName(variableName), expression(expression) {}

	VariableValue evaluate(Environment& env) const override;

	~AssignmentNode()override;
};


// Number Node: Represents a numerical literal
struct NumberNode : public ASTNode {
	double value;

	explicit NumberNode(double value) : value(value) {}

	VariableValue evaluate(Environment& env) const override {
		return value;
	}

	~NumberNode() override = default;
};

// Function Definition Node: Represents a user-defined function
struct FunctionNode : public ASTNode {
	std::string name;
	ValueType returnType;
	std::vector<std::pair<std::string, ValueType>> parameters;
	ASTNode* body;

	explicit FunctionNode(const std::string& name, ValueType returnType,
		const std::vector<std::pair<std::string, ValueType>>& parameters, ASTNode* body)
		: name(name), returnType(returnType), parameters(parameters), body(body) {}

	VariableValue evaluate(Environment& env) const override;

	~FunctionNode() override;
};

// Function Call Node: Represents calling a function with arguments
struct FunctionCallNode : public ASTNode {
	std::string name;
	std::vector<ASTNode*> arguments;

	explicit FunctionCallNode(const std::string& name, const std::vector<ASTNode*>& arguments)
		: name(name), arguments(arguments) {}

	VariableValue evaluate(Environment& env) const override;

	VariableValue evaluateArg(ASTNode* arg, Environment& env) const;

	~FunctionCallNode() override;
};

// Return Node: Represents a return statement in a function
struct ReturnNode : public ASTNode {
	ASTNode* returnValue;

	explicit ReturnNode(ASTNode* returnValue) : returnValue(returnValue) {}

	VariableValue evaluate(Environment& env) const override;

	~ReturnNode() override;
};

// If Node: Represents an if-else statement
struct IfNode : public ASTNode {
	ASTNode* condition;
	ASTNode* thenBranch;
	ASTNode* elseBranch;

	explicit IfNode(ASTNode* condition, ASTNode* thenBranch, ASTNode* elseBranch = nullptr)
		: condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

	VariableValue evaluate(Environment& env) const override;

	~IfNode() override;
};

// While Node: Represents a while loop
struct WhileNode : public ASTNode {
	ASTNode* condition;
	ASTNode* body;

	explicit WhileNode(ASTNode* condition, ASTNode* body) : condition(condition), body(body) {}

	VariableValue evaluate(Environment& env) const override;

	~WhileNode() override;
};

// Binary Operation Node: Represents a binary operation like +, -, *, /
struct BinaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* left;
	ASTNode* right;

	explicit BinaryOpNode(TokenType op, ASTNode* left, ASTNode* right)
		: op(op), left(left), right(right) {}

	VariableValue evaluate(Environment& env) const override;

	~BinaryOpNode() override;
};

// Unary Operation Node: Represents a unary operation like - or !
struct UnaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* operand;

	explicit UnaryOpNode(TokenType op, ASTNode* operand) : op(op), operand(operand) {}

	VariableValue evaluate(Environment& env) const override;

	~UnaryOpNode() override;
};

