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



class Environment {
private:
	std::unordered_map<std::string, ScriptFunction> functionRegistry;
	std::unordered_map<std::string, ASTNode*> userFunctionRegistry;
	std::unordered_map<std::string, std::pair<VariableValue, ValueType>> variableTable;

public:
	Environment() = default;
	~Environment() = default;

	// Register a built-in C++ function by name
	void registerFunction(const std::string& name, ScriptFunction func);

	// Register a user-defined function (AST-based)
	void registerUserFunction(const std::string& name, ASTNode* functionNode);

	// Evaluate a function by name with given arguments
	double evaluateFunction(const std::string& name, const std::vector<double>& args) const;

	// Declare a variable by name and type
	void declareVariable(const std::string& name, ValueType type);

	void setVariable(const std::string& name, const VariableValue& value);

	// Get a variable's value
	VariableValue getVariable(const std::string& name) const;
	
};



