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
	virtual VariableValue evaluate(Environment& env) = 0; // can return the const here
};

// Program Node: Represents a collection of statements
struct ProgramNode : public ASTNode {
	std::vector<ASTNode*> statements;

	explicit ProgramNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	VariableValue evaluate(Environment& env) override;

	~ProgramNode() override;
};

struct IncrementNode : public ASTNode {
	IncrementType incrementType;
	std::string variableName;

	explicit IncrementNode(IncrementType type, const std::string& varName)
		: incrementType(type), variableName(varName) {}

	VariableValue evaluate(Environment& env) override;
	~IncrementNode() override = default;

};

struct ForNode : public ASTNode {
	ASTNode* initializer;  // Variable declaration or assignment
	ASTNode* condition;    // Loop condition
	ASTNode* update;       // Update expression
	ASTNode* body;         // Loop body

	explicit ForNode(ASTNode* initializer, ASTNode* condition, ASTNode* update, ASTNode* body)
		: initializer(initializer), condition(condition), update(update), body(body) {}

	VariableValue evaluate(Environment& env) override;

	~ForNode() override;
};

struct BooleanNode : public ASTNode {
	bool value;

	explicit BooleanNode(bool value) : value(value) {}

	VariableValue evaluate(Environment& env) override;

	~BooleanNode() override = default;
};


struct StringNode : public ASTNode {
	std::string value;

	explicit StringNode(const std::string& value) : value(value) {}

	VariableValue evaluate(Environment& env) override;

	~StringNode() override = default;
};


struct DoWhileNode : public ASTNode {
	ASTNode* body;        // Loop body
	ASTNode* condition;   // Loop condition

	explicit DoWhileNode(ASTNode* body, ASTNode* condition)
		: body(body), condition(condition) {}

	VariableValue evaluate(Environment& env) override;

	~DoWhileNode() override;
};


// Block Node: Represents a block of statements, typically for control flow or functions
struct BlockNode : public ASTNode {
	std::vector<ASTNode*> statements;

	explicit BlockNode(const std::vector<ASTNode*>& statements) : statements(statements) {}

	VariableValue evaluate(Environment& env) override;

	~BlockNode() override;
};

// Variable Node: Represents a reference to a variable
struct VariableNode : public ASTNode {
	std::string name;

	explicit VariableNode(const std::string& name) : name(name) {}

	VariableValue evaluate(Environment& env) override;

	std::string getName() const;

	~VariableNode() override = default;
};

// Declaration Node: Represents a variable declaration, possibly with an initializer
struct DeclarationNode : public ASTNode {
	std::string variableName;
	ValueType type;
	ASTNode* initializer;

	explicit DeclarationNode(const std::string& variableName, ValueType type, ASTNode* initializer = nullptr)
		: variableName(variableName), type(type), initializer(initializer) {}

	VariableValue evaluate(Environment& env) override;

	~DeclarationNode() override;
};

// Assignment Node: Represents assigning a value to a variable
struct AssignmentNode : public ASTNode {
	std::string variableName;
	ASTNode* index;
	ASTNode* expression;

	explicit AssignmentNode(const std::string& variableName, ASTNode* expression)
		: variableName(variableName), index(nullptr), expression(expression) {}

	explicit AssignmentNode(const std::string& variableName, ASTNode* index, ASTNode* expression)
		: variableName(variableName), index(index), expression(expression) {}

	VariableValue evaluate(Environment& env) override;

	~AssignmentNode()override;
};


// Number Node: Represents a numerical literal
struct NumberNode : public ASTNode {
	double value;

	explicit NumberNode(double value) : value(value) {}

	VariableValue evaluate(Environment& env) override {
		return VariableValue(value);
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

	VariableValue evaluate(Environment& env) override;

	~FunctionNode() override;
};

// Function Call Node: Represents calling a function with arguments
struct FunctionCallNode : public ASTNode {
	std::string name;
	std::vector<ASTNode*> arguments;
	std::vector<std::string> argumentsNames;

	explicit FunctionCallNode(const std::string& name, const std::vector<ASTNode*>& arguments)
		: name(name), arguments(arguments) {}

	VariableValue evaluate(Environment& env) override;

	~FunctionCallNode() override;
};

// Return Node: Represents a return statement in a function
struct ReturnNode : public ASTNode {
	ASTNode* returnValue;

	explicit ReturnNode(ASTNode* returnValue) : returnValue(returnValue) {}

	VariableValue evaluate(Environment& env) override;

	~ReturnNode() override;
};

// If Node: Represents an if-else statement
struct IfNode : public ASTNode {
	ASTNode* condition;
	ASTNode* thenBranch;
	ASTNode* elseBranch;

	explicit IfNode(ASTNode* condition, ASTNode* thenBranch, ASTNode* elseBranch = nullptr)
		: condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

	VariableValue evaluate(Environment& env) override;

	~IfNode() override;
};

// While Node: Represents a while loop
struct WhileNode : public ASTNode {
	ASTNode* condition;
	ASTNode* body;

	explicit WhileNode(ASTNode* condition, ASTNode* body) : condition(condition), body(body) {}

	VariableValue evaluate(Environment& env) override;

	~WhileNode() override;
};

// Binary Operation Node: Represents a binary operation like +, -, *, /
struct BinaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* left;
	ASTNode* right;

	explicit BinaryOpNode(TokenType op, ASTNode* left, ASTNode* right)
		: op(op), left(left), right(right) {}

	VariableValue evaluate(Environment& env) override;

	~BinaryOpNode() override;
};

// Unary Operation Node: Represents a unary operation like - or !
struct UnaryOpNode : public ASTNode {
	TokenType op;
	ASTNode* operand;

	explicit UnaryOpNode(TokenType op, ASTNode* operand) : op(op), operand(operand) {}

	VariableValue evaluate(Environment& env) override;

	~UnaryOpNode() override;
};

struct ArrayNode : public ASTNode {
	std::vector<ASTNode*> elements;

	explicit ArrayNode(const std::vector<ASTNode*>& elements)
		: elements(elements) {}

	// Evaluate the array elements and return them as a VariableValue
	VariableValue evaluate(Environment& env) override;

	~ArrayNode() override;
};

struct MapNode : public ASTNode {
	std::unordered_map<std::string, ASTNode*> elements;

	explicit MapNode(const std::unordered_map<std::string, ASTNode*>& elements)
		: elements(elements) {}

	// Evaluate the map elements and return them as a VariableValue
	VariableValue evaluate(Environment& env) override;

	~MapNode() override;
};

struct IndexNode : public ASTNode {
	std::string variableName;
	ASTNode* indexExpression;

	IndexNode(const std::string& name, ASTNode* indexExpr)
		: variableName(name), indexExpression(indexExpr) {}

	~IndexNode() override {
		delete indexExpression;
	}

	VariableValue evaluate(Environment& env) override;
};


struct ClassDefinitionNode : public ASTNode {
	std::string className;
	std::unordered_map<std::string, ASTNode*> members;
	FunctionNode* constructor; // Optional constructor

	explicit ClassDefinitionNode(const std::string& className, const std::unordered_map<std::string, ASTNode*>& members, FunctionNode* constructor = nullptr)
		: className(className), members(members), constructor(constructor) {}

	VariableValue evaluate(Environment& env);

	~ClassDefinitionNode() override {
		for (auto& [_, member] : members) {
			delete member;
		}
	}
};


struct ObjectInstantiationNode : public ASTNode {
	std::string className;
	std::vector<ASTNode*> constructorArgs;

	explicit ObjectInstantiationNode(const std::string& className, const std::vector<ASTNode*>& args)
		: className(className), constructorArgs(args) {}

	VariableValue evaluate(Environment& env) override;

	~ObjectInstantiationNode() override {
		for (ASTNode* arg : constructorArgs) {
			delete arg;
		}
	}
};


struct MemberAccessNode : public ASTNode {
	ASTNode* object;
	std::string memberName;

	explicit MemberAccessNode(ASTNode* object, const std::string& memberName)
		: object(object), memberName(memberName) {}

	VariableValue evaluate(Environment& env) override;

	~MemberAccessNode() override {
		delete object;
	}
};



