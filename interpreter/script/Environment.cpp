#include "Environment.hpp"
#include "AST.hpp"

Environment::Environment()
{
	// Initialize with a global scope
	variableScopes.emplace_back();
}

// Register a built-in C++ function by name
void Environment::registerFunction(const std::string& name, ScriptFunction func) {
	if (functionRegistry.contains(name)) {
		throw std::runtime_error("Function already registered: " + name);
	}
	functionRegistry[name] = func;
}

// Register a user-defined function (AST-based)
void Environment::registerUserFunction(const std::string& name, ASTNode* functionNode) {
	if (userFunctionRegistry.contains(name)) {
		throw std::runtime_error("User-defined function already registered: " + name);
	}
	userFunctionRegistry[name] = functionNode;
}

VariableValue Environment::evaluateFunction(const std::string& name,const std::vector<VariableValue>& args,const std::vector<std::string>& argumentsNames) {
	if (functionRegistry.contains(name)) {
		return functionRegistry.at(name)(args, argumentsNames, *this);
	}

	if (userFunctionRegistry.contains(name)) {
		auto* functionNode = dynamic_cast<FunctionNode*>(userFunctionRegistry.at(name));
		if (!functionNode) {
			throw std::runtime_error("Function " + name + " is not properly defined.");
		}

		if (args.size() != functionNode->parameters.size()) {
			throw std::runtime_error("Function " + name + " expects " +
				std::to_string(functionNode->parameters.size()) +
				" arguments, but got " + std::to_string(args.size()));
		}

		// Create a new local scope for this function call
		pushScope();

		// Assign arguments to the parameters in the new scope
		for (size_t i = 0; i < args.size(); ++i) {
			const auto& param = functionNode->parameters[i];
			declareVariable(param.first, param.second);
			setVariable(param.first, args[i]);
		}

		// Execute the function body
		VariableValue returnValue = functionNode->body->evaluate(*this);


		// Remove the local scope after the function completes
		popScope();

		return returnValue;
	}

	throw std::runtime_error("Undefined function: " + name);
}

void Environment::registerClass(const std::string& name, ClassDefinitionNode* classDef)
{
	if (classRegistry.contains(name)) {
		throw std::runtime_error("Class already registered: " + name);
	}
	classRegistry[name] = classDef;
}

VariableValue Environment::instantiateObject(const std::string& className, const std::vector<ASTNode*>& args)
{
	// Check if the class is defined in the registry
	if (!classRegistry.contains(className)) {
		throw std::runtime_error("Undefined class: " + className);
	}

	// Get the class definition from the registry
	ClassDefinitionNode* classDef = classRegistry.at(className);
	std::unordered_map<std::string, VariableValue> objectMembers;

	// Initialize member variables
	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto declNode = dynamic_cast<DeclarationNode*>(memberNode)) {
			// Evaluate the member variable to initialize it
			objectMembers[memberName] = declNode->evaluate(*this);
		}
	}

	// Add member functions to the object
	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto funcNode = dynamic_cast<FunctionNode*>(memberNode)) {
			// Store the function node as a callable member of the object
			objectMembers[memberName] = VariableValue(funcNode);
		}
	}

	// Call constructor if it exists
	if (classDef->constructor) {
		// Evaluate constructor arguments
		std::vector<VariableValue> evaluatedArgs;
		for (ASTNode* arg : args) {
			evaluatedArgs.push_back(arg->evaluate(*this));
		}

		// Push a new scope for the constructor
		pushScope();

		// Set constructor parameters in the new scope
		for (size_t i = 0; i < classDef->constructor->parameters.size(); ++i) {
			const auto& param = classDef->constructor->parameters[i];
			if (i < evaluatedArgs.size()) {
				declareVariable(param.first, param.second);
				setVariable(param.first, evaluatedArgs[i]);
			}
			else {
				throw std::runtime_error("Insufficient arguments provided for constructor of class: " + className);
			}
		}

		// Execute the constructor body
		classDef->constructor->body->evaluate(*this);

		// Pop the constructor scope
		popScope();
	}

	// Return the created object, which now includes both variables and functions
	return VariableValue(objectMembers); // Objects are represented as maps of their members
}

bool Environment::isClassDefined(const std::string& identifier)
{
	return classRegistry.contains(identifier);
}



// Declare a variable by name and type
void Environment::declareVariable(const std::string& name, ValueType type) {
	if (variableScopes.back().contains(name)) {
		throw std::runtime_error("Variable already declared: " + name);
	}

	VariableValue defaultValue;
	switch (type) {
	case ValueType::INT:
	case ValueType::FLOAT:
		defaultValue = VariableValue(0.0);
		break;
	case ValueType::BOOL:
		defaultValue = VariableValue(false);
		break;
	case ValueType::STRING:
		defaultValue = VariableValue("");
		break;
	case ValueType::ARRAY:
		defaultValue = VariableValue(std::vector<VariableValue>());  // Default to an empty array
		break;
	case ValueType::MAP:
		defaultValue = VariableValue(std::unordered_map<std::string, VariableValue>());  // Default to an empty map
		break;
	default:
		throw std::runtime_error("Unsupported variable type for declaration.");
	}

	variableTable[name] = std::make_pair(defaultValue, type);
}

void Environment::setVariable(const std::string& name, const VariableValue& value) {
	
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			scope->at(name).first = value;
			return;
		}
	}
	//if not in scope then global
	if (variableTable.contains(name)) {
		variableTable.at(name).first = value;
		return;
	}
	throw std::runtime_error("Undefined variable: " + name);
}

void Environment::declareObject(const std::string& className, const std::string& objectName)
{
	// Check if the class is defined in the registry
	if (!classRegistry.contains(className)) {
		throw std::runtime_error("Undefined class: " + className);
	}

	// Get the class definition
	ClassDefinitionNode* classDef = classRegistry.at(className);

	// Initialize the members of the object based on the class definition
	std::unordered_map<std::string, VariableValue> objectMembers;

	// Add member variables
	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto declNode = dynamic_cast<DeclarationNode*>(memberNode)) {
			// Evaluate the declaration to get the initial value
			VariableValue initialValue = declNode->evaluate(*this);
			objectMembers[memberName] = initialValue;
		}
	}

	// Add member functions
	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto funcNode = dynamic_cast<FunctionNode*>(memberNode)) {
			// Store function pointers or function nodes directly in the object map
			objectMembers[memberName] = VariableValue(funcNode);
		}
	}

	// Create a VariableValue to represent the object (use a map to store its members)
	VariableValue objectValue(objectMembers);

	// Add the new object to the current scope
	if (variableScopes.empty()) {
		throw std::runtime_error("No active scope to declare object in.");
	}

	variableScopes.back()[objectName] = std::make_pair(objectValue, ValueType::MAP);
}

bool Environment::isVariableDeclared(const std::string& name)
{
	// Check in the current scope if the variable is already declared
	if (!variableScopes.empty()) {
		const auto& currentScope = variableScopes.back();
		return currentScope.find(name) != currentScope.end();
	}
	return false;
}

bool Environment::isMemberFunction(const std::unordered_map<std::string, VariableValue>& objMap, const std::string& methodName)
{
	// Check if the object contains the member and if it refers to a function
	auto it = objMap.find(methodName);
	if (it != objMap.end()) {
		// Check if the value is a FunctionNode (which indicates it's callable)
		return std::holds_alternative<FunctionNode*>(it->second.value);
	}
	return false;
}

VariableValue Environment::callMemberFunction(const std::unordered_map<std::string, VariableValue>& objMap, const std::string& methodName, const std::vector<VariableValue>& args)
{
	// Check if the object has the method
	auto it = objMap.find(methodName);
	if (it == objMap.end()) {
		throw std::runtime_error("Undefined member function: " + methodName);
	}

	// Ensure the member is a function
	FunctionNode* functionNode = std::get<FunctionNode*>(it->second.value);
	if (!functionNode) {
		throw std::runtime_error("Member " + methodName + " is not callable.");
	}

	// Create a new scope for the function call
	pushScope();

	// Assign arguments to function parameters in the new scope
	for (size_t i = 0; i < functionNode->parameters.size(); ++i) {
		const auto& param = functionNode->parameters[i];
		if (i < args.size()) {
			declareVariable(param.first, param.second);
			setVariable(param.first, args[i]);
		}
		else {
			throw std::runtime_error("Insufficient arguments provided for function: " + methodName);
		}
	}

	// Execute the function body
	VariableValue returnValue = functionNode->body->evaluate(*this);

	// Pop the function scope
	popScope();

	return returnValue;
}

// Get a variable's value
VariableValue Environment::getVariable(const std::string& name) {
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			return scope->at(name).first;
		}
	}
	//if not in scope then global
	if (variableTable.contains(name)) {
		return variableTable.at(name).first;
	}

	throw std::runtime_error("Undefined variable: " + name);
}

void Environment::pushScope()
{

	variableScopes.emplace_back();

}
