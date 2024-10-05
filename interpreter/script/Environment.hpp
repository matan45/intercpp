#pragma once

#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <utility>   
#include <vector>

#include "Types.hpp"


struct ASTNode;
struct ClassDefinitionNode;



class Environment {
private:
	std::unordered_map<std::string, ScriptFunction> functionRegistry;
	std::unordered_map<std::string, ASTNode*> userFunctionRegistry;
	std::unordered_map<std::string, ClassDefinitionNode*> classRegistry;
	std::unordered_map<std::string, std::pair<VariableValue, ValueType>> variableTable;
	std::vector<std::unordered_map<std::string, std::pair<VariableValue, ValueType>>> variableScopes;

public:
	explicit Environment();
	~Environment() = default;

	// Register a built-in C++ function by name
	void registerFunction(const std::string& name, ScriptFunction func);

	// Register a user-defined function (AST-based)
	void registerUserFunction(const std::string& name, ASTNode* functionNode);
	
	// Evaluate a function by name with given arguments
	VariableValue evaluateFunction(const std::string& name,const std::vector<VariableValue>& args,const std::vector<std::string>& argumentsNames = std::vector<std::string>());

	void registerClass(const std::string& name, ClassDefinitionNode* classDef);
	VariableValue instantiateObject(const std::string& className, const std::vector<ASTNode*>& args);

	// Declare a variable by name and type
	void declareVariable(const std::string& name, ValueType type);

	void setVariable(const std::string& name, const VariableValue& value);

	// Get a variable's value
	VariableValue getVariable(const std::string& name);

	// Add a new scope (for entering a function)
	void pushScope();

	// Remove the current scope (for exiting a function)
	void popScope() {
		if (variableScopes.size() <= 1) {
			throw std::runtime_error("Cannot pop the global scope.");
		}
		variableScopes.pop_back();
	}
	
};



