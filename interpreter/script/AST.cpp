#include "AST.hpp"
#include "Environment.hpp"

VariableValue ProgramNode::evaluate(Environment& env) const {
	for (ASTNode const* statement : statements) {
		statement->evaluate(env);
	}
	return VariableValue(); // Program as a whole doesn’t return a specific value
}

ProgramNode::~ProgramNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

VariableValue ForNode::evaluate(Environment& env) const {
	// Evaluate the initializer (e.g., int i = 0)
	if (initializer) {
		initializer->evaluate(env);
	}

	// Loop until the condition evaluates to false
	while (true) {
		// Evaluate the loop condition
		VariableValue condValue = condition->evaluate(env);

		// Use std::get_if to check if the condition is a boolean
		if (auto boolPtr = std::get_if<bool>(&condValue.value)) {
			if (!*boolPtr) {
				break;  // Exit the loop if the condition evaluates to false
			}
		}
		// If the condition is not a boolean but could be something like an integer or double,
		// handle it in a boolean-compatible way.
		else if (auto doublePtr = std::get_if<double>(&condValue.value)) {
			if (*doublePtr == 0.0) {
				break;  // If the numeric value is 0, exit the loop (considered false)
			}
		}
		else {
			throw std::runtime_error("Condition for 'for' must evaluate to a boolean or numeric value.");
		}

		// Execute the body of the loop
		body->evaluate(env);

		// Execute the update expression (e.g., i++)
		if (update) {
			update->evaluate(env);
		}
	}

	// Return a "null" or default value to indicate no specific value from the for loop
	return VariableValue();  // Explicitly return a "null" to indicate absence of meaningful value
}

ForNode::~ForNode() {
	delete initializer;
	delete condition;
	delete update;
	delete body;
}

VariableValue BooleanNode::evaluate(Environment& env) const {
	// Return 1.0 for true, 0.0 for false (to allow compatibility in numeric contexts)
	return VariableValue(value);
}

VariableValue StringNode::evaluate(Environment& env) const {
	// Return the string value contained in this node.
	return VariableValue(value);
}

VariableValue DoWhileNode::evaluate(Environment& env) const {
	do {
		body->evaluate(env);
		VariableValue condValue = condition->evaluate(env);

		// Ensure the condition is boolean
		if (auto boolPtr = std::get_if<bool>(&condValue.value)) {
			if (!*boolPtr) {
				break;  // Exit the loop if condition is false
			}
		}
		else {
			throw std::runtime_error("Condition for 'do-while' must be a boolean.");
		}

	} while (true);

	return VariableValue();  // No meaningful value to return for a do-while loop
}

DoWhileNode::~DoWhileNode() {
	delete body;
	delete condition;
}

VariableValue IncrementNode::evaluate(Environment& env) const {
	// Get the current value of the variable from the environment
	VariableValue value = env.getVariable(variableName);

	// Ensure the value is numeric (double)
	if (!std::holds_alternative<double>(value.value)) {
		throw std::runtime_error("Increment operation only supports numeric types.");
	}

	// Get a reference to the double value for in-place modification
	double& varValue = std::get<double>(value.value);
	double originalValue = varValue;  // Save the original value for postfix increment

	VariableValue result;  // Variable to hold the return value

	// Perform increment operation
	if (incrementType == IncrementType::PREFIX) {
		// Prefix: increment first, then return the updated value
		++varValue;
		result = VariableValue(varValue);
	}
	else {
		// Postfix: return the original value, then increment
		result = VariableValue(originalValue);
		varValue++;
	}

	// Update the variable in the environment with the modified value
	env.setVariable(variableName, value);

	// Return the result of the increment operation (either updated or original value)
	return result;
}

VariableValue BlockNode::evaluate(Environment& env) const {
	VariableValue lastValue;
	for (const auto& statement : statements) {
		lastValue = statement->evaluate(env);
	}
	return lastValue;
}

BlockNode::~BlockNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

VariableValue UnaryOpNode::evaluate(Environment& env) const {
	// Evaluate the operand
	VariableValue operandValue = operand->evaluate(env);

	switch (op) {
	case TokenType::MINUS: {
		// Apply unary minus, only if the operand is numeric (double)
		if (auto doublePtr = std::get_if<double>(&operandValue.value)) {
			return VariableValue(-(*doublePtr));
		}
		else {
			throw std::runtime_error("Unary minus can only be applied to numeric values.");
		}
	}
	case TokenType::NOT: {
		// Apply logical NOT, only if the operand is boolean
		if (auto boolPtr = std::get_if<bool>(&operandValue.value)) {
			return VariableValue(!(*boolPtr));
		}
		else {
			throw std::runtime_error("Logical NOT can only be applied to boolean values.");
		}
	}
	default:
		throw std::runtime_error("Unsupported unary operation.");
	}
}

UnaryOpNode::~UnaryOpNode() {
	delete operand;
}

VariableValue BinaryOpNode::evaluate(Environment& env) const {
	VariableValue leftValue = left->evaluate(env);
	VariableValue rightValue = right->evaluate(env);

	// Handle numeric operations (+, -, *, /)
	if (auto leftDouble = std::get_if<double>(&leftValue.value)) {
		if (auto rightDouble = std::get_if<double>(&rightValue.value)) {
			switch (op) {
			case TokenType::PLUS:
				return VariableValue(*leftDouble + *rightDouble);
			case TokenType::MINUS:
				return VariableValue(*leftDouble - *rightDouble);
			case TokenType::MULTIPLY:
				return VariableValue(*leftDouble * *rightDouble);
			case TokenType::DIVIDE:
				if (*rightDouble == 0) {
					throw std::runtime_error("Division by zero.");
				}
				return VariableValue(*leftDouble / *rightDouble);
			case TokenType::LESS:
				return VariableValue(*leftDouble < *rightDouble);
			case TokenType::LESS_EQUALS:
				return VariableValue(*leftDouble <= *rightDouble);
			case TokenType::GREATER:
				return VariableValue(*leftDouble > *rightDouble);
			case TokenType::GREATER_EQUALS:
				return VariableValue(*leftDouble >= *rightDouble);
			default:
				throw std::runtime_error("Unsupported binary operation for doubles.");
			}
		}
	}

	// Handle string concatenation (+)
	if (auto leftStr = std::get_if<std::string>(&leftValue.value)) {
		if (auto rightStr = std::get_if<std::string>(&rightValue.value)) {
			if (op == TokenType::PLUS) {
				return VariableValue(*leftStr + *rightStr);
			}
			throw std::runtime_error("Unsupported operation for strings.");
		}
	}

	// Handle logical operations (&&, ||)
	if (op == TokenType::AND || op == TokenType::OR) {
		if (auto leftBool = std::get_if<bool>(&leftValue.value)) {
			if (auto rightBool = std::get_if<bool>(&rightValue.value)) {
				if (op == TokenType::AND) {
					return VariableValue(*leftBool && *rightBool);
				}
				else if (op == TokenType::OR) {
					return VariableValue(*leftBool || *rightBool);
				}
			}
		}
		else {
			throw std::runtime_error("Logical operations require boolean values.");
		}
	}

	// Handle equality and inequality operations (==, !=)
	if (op == TokenType::EQUALS || op == TokenType::NOT_EQUALS) {
		if (leftValue.value.index() == rightValue.value.index()) {
			bool result = false;

			if (auto leftDouble = std::get_if<double>(&leftValue.value)) {
				auto rightDouble = std::get_if<double>(&rightValue.value);
				result = (*leftDouble == *rightDouble);
			}
			else if (auto leftBool = std::get_if<bool>(&leftValue.value)) {
				auto rightBool = std::get_if<bool>(&rightValue.value);
				result = (*leftBool == *rightBool);
			}
			else if (auto leftStr = std::get_if<std::string>(&leftValue.value)) {
				auto rightStr = std::get_if<std::string>(&rightValue.value);
				result = (*leftStr == *rightStr);
			}

			if (op == TokenType::EQUALS) {
				return VariableValue(result);
			}
			else if (op == TokenType::NOT_EQUALS) {
				return VariableValue(!result);
			}
		}
		else {
			throw std::runtime_error("Cannot compare different types for equality.");
		}
	}

	throw std::runtime_error("Unsupported types for binary operation.");
}


BinaryOpNode::~BinaryOpNode() {
	delete left;
	delete right;
}

VariableValue WhileNode::evaluate(Environment& env) const {
	while (true) {
		VariableValue condValue = condition->evaluate(env);

		// Ensure the condition is boolean
		if (auto boolPtr = std::get_if<bool>(&condValue.value)) {
			if (!*boolPtr) {
				break;  // Exit the loop if condition is false
			}
		}
		else {
			throw std::runtime_error("Condition for 'while' must be a boolean.");
		}

		body->evaluate(env);
	}

	return VariableValue();  // No meaningful value to return for a while loop
}

WhileNode::~WhileNode() {
	delete condition;
	delete body;
}


VariableValue IfNode::evaluate(Environment& env) const {
	VariableValue condValue = condition->evaluate(env);

	// Ensure the condition is boolean
	if (auto boolPtr = std::get_if<bool>(&condValue.value)) {
		if (*boolPtr) {
			return thenBranch->evaluate(env);
		}
		else if (elseBranch != nullptr) {
			return elseBranch->evaluate(env);
		}
	}
	else {
		throw std::runtime_error("Condition for 'if' must be a boolean.");
	}

	return VariableValue();  // Return an empty value if no branch is executed
}

IfNode::~IfNode() {
	delete condition;
	delete thenBranch;
	delete elseBranch;
}

VariableValue ReturnNode::evaluate(Environment& env) const {
	VariableValue result = returnValue->evaluate(env);

	// Determine the type of result and print it accordingly
	std::cout << "ReturnNode: Returning value ";
	if (auto doublePtr = std::get_if<double>(&result.value)) {
		std::cout << *doublePtr << std::endl;
	}
	else if (auto boolPtr = std::get_if<bool>(&result.value)) {
		std::cout << (*boolPtr ? "true" : "false") << std::endl;
	}
	else if (auto strPtr = std::get_if<std::string>(&result.value)) {
		std::cout << *strPtr << std::endl;
	}
	else {
		std::cout << "unknown type" << std::endl;
	}

	return result;
}

ReturnNode::~ReturnNode() {
	delete returnValue;
}

VariableValue FunctionCallNode::evaluate(Environment& env) const {
	// Evaluate all arguments and collect them
	std::vector<VariableValue> evaluatedArgs;
	for (ASTNode const* arg : arguments) {
		evaluatedArgs.push_back(arg->evaluate(env));
	}

	// Call the function with the arguments and return the result
	VariableValue result = env.evaluateFunction(name, evaluatedArgs);

	return result;
}


FunctionCallNode::~FunctionCallNode() {
	for (ASTNode* arg : arguments) {
		delete arg;
	}
}

VariableValue FunctionNode::evaluate(Environment& env) const {
	// Execute the function body

	return VariableValue(); // Default return if nothing returned
}

FunctionNode::~FunctionNode() {
	delete body;
}

VariableValue DeclarationNode::evaluate(Environment& env) const {
	// Declare the variable in the current scope with a default value
	VariableValue defaultValue;

	switch (type) {
	case ValueType::BOOL:
		defaultValue.value = false;  // Boolean variables default to false
		break;
	case ValueType::INT:
		defaultValue.value = 0.0;  // Integer variables default to 0
		break;
	case ValueType::FLOAT:
		defaultValue.value = 0.0;  // Float variables also default to 0.0
		break;
	case ValueType::STRING:
		defaultValue.value = "";  // String variables default to an empty string
		break;
	case ValueType::ARRAY:
		defaultValue.value = std::vector<VariableValue>();  // Arrays default to an empty vector
		break;
	case ValueType::MAP:
		defaultValue.value = std::unordered_map<std::string, VariableValue>();  // Maps default to an empty map
		break;
	default:
		throw std::runtime_error("Unknown variable type for declaration: " + variableName);
	}

	// Declare the variable in the environment
	env.declareVariable(variableName, type);

	// If there is an initializer, evaluate it and set the value in the environment
	if (initializer) {
		VariableValue value = initializer->evaluate(env);
		env.setVariable(variableName, value);
	}
	else {
		// Set the default value in the environment
		env.setVariable(variableName, defaultValue);
	}

	return env.getVariable(variableName);  // Return the value of the declared variable
}

DeclarationNode::~DeclarationNode() {
	delete initializer;
}

VariableValue AssignmentNode::evaluate(Environment& env) const {
	if (index == nullptr) {
		// Regular variable assignment
		VariableValue value = expression->evaluate(env);
		env.setVariable(variableName, value);
		return value;  // Return the assigned value
	}
	else {
		// Array or Map element assignment
		VariableValue container = env.getVariable(variableName);
		VariableValue indexValue = index->evaluate(env);
		VariableValue newValue = expression->evaluate(env);

		if (auto arrayPtr = std::get_if<std::vector<VariableValue>>(&container.value)) {
			if (auto indexPtr = std::get_if<double>(&indexValue.value)) {
				auto idx = static_cast<int>(*indexPtr);
				if (idx >= 0 && idx < arrayPtr->size()) {
					(*arrayPtr)[idx] = newValue;
					env.setVariable(variableName, VariableValue(*arrayPtr));  // Update the array in the environment
				}
				else {
					throw std::runtime_error("Array index out of bounds.");
				}
			}
			else {
				throw std::runtime_error("Array index must be an integer.");
			}
		}
		else if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&container.value)) {
			if (auto keyPtr = std::get_if<std::string>(&indexValue.value)) {
				(*mapPtr)[*keyPtr] = newValue;
				env.setVariable(variableName, VariableValue(*mapPtr));  // Update the map in the environment
			}
			else {
				throw std::runtime_error("Map key must be a string.");
			}
		}
		else {
			throw std::runtime_error("Cannot index non-array or non-map type.");
		}

		return newValue;
	}
}

AssignmentNode::~AssignmentNode() {
	delete expression;
	if (index) delete index;
}

VariableValue VariableNode::evaluate(Environment& env) const {
	auto value = env.getVariable(name);
	if (std::holds_alternative<double>(value.value)) {
		double numValue = std::get<double>(value.value);
		std::cout << "VariableNode: Retrieved numeric value " << numValue << " for variable " << name << std::endl;
		return VariableValue(numValue);
	}
	else if (std::holds_alternative<bool>(value.value)) {
		bool boolValue = std::get<bool>(value.value);
		std::cout << "VariableNode: Retrieved boolean value " << (boolValue ? "true" : "false") << " for variable " << name << std::endl;
		return VariableValue(boolValue);
	}
	// Handle string type: return a default value or throw an error
	else if (std::holds_alternative<std::string>(value.value)) {
		std::string strValue = std::get<std::string>(value.value);
		std::cout << "VariableNode: Retrieved string value \"" << strValue << "\" for variable " << name << std::endl;
		// Option 1: Return a default value such as 0.0
		return VariableValue(strValue);
	}
	// Handle array type (std::vector<VariableValue>)
	if (auto vecPtr = std::get_if<std::vector<VariableValue>>(&value.value)) {
		std::cout << "VariableNode: Retrieved array value of size " << vecPtr->size() << " for variable " << name << std::endl;
		return VariableValue(*vecPtr);
	}

	// Handle map type (std::unordered_map<std::string, VariableValue>)
	if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&value.value)) {
		std::cout << "VariableNode: Retrieved map value with " << mapPtr->size() << " entries for variable " << name << std::endl;
		return VariableValue(*mapPtr);
	}
	else {
		throw std::runtime_error("Expected numeric value for variable: " + name);
	}
}

// Evaluate the array elements and return them as a VariableValue
VariableValue ArrayNode::evaluate(Environment& env) const {
	std::vector<VariableValue> evaluatedElements;

	for (ASTNode const* element : elements) {
		evaluatedElements.push_back(element->evaluate(env));
	}

	return VariableValue(evaluatedElements);
}

ArrayNode::~ArrayNode() {
	for (ASTNode* element : elements) {
		delete element;
	}
}

VariableValue MapNode::evaluate(Environment& env) const {
	std::unordered_map<std::string, VariableValue> evaluatedElements;

	for (const auto& [key, valueNode] : elements) {
		evaluatedElements[key] = valueNode->evaluate(env);
	}

	return VariableValue(evaluatedElements);
}

MapNode::~MapNode() {
	for (const auto& [key, valueNode] : elements) {
		delete valueNode;
	}
}

VariableValue IndexNode::evaluate(Environment& env) const {
	// Retrieve the container (array or map) from the environment
	VariableValue containerValue = env.getVariable(variableName);

	// Evaluate the index expression
	VariableValue indexValue = indexExpression->evaluate(env);

	// Handle indexing for arrays
	if (auto arrayPtr = std::get_if<std::vector<VariableValue>>(&containerValue.value)) {
		// Ensure the index is numeric
		if (auto indexDouble = std::get_if<double>(&indexValue.value)) {
			int index = static_cast<int>(*indexDouble);
			if (index >= 0 && index < arrayPtr->size()) {
				return VariableValue((*arrayPtr)[index].value);  // Return the element at the given index
			}
			else {
				throw std::runtime_error("Array index out of bounds for variable: " + variableName);
			}
		}
		else {
			throw std::runtime_error("Array indexing requires a numeric index.");
		}
	}

	// Handle indexing for maps
	if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&containerValue.value)) {
		if (auto strPtr = std::get_if<std::string>(&indexValue.value)) {
			auto it = mapPtr->find(*strPtr);
			if (it != mapPtr->end()) {
				return VariableValue(it->second.value);  // Return the value corresponding to the key
			}
			else {
				throw std::runtime_error("Key not found in map: " + *strPtr);
			}
		}
		else {
			throw std::runtime_error("Map indexing requires a string key.");
		}
	}

	// If the container is neither an array nor a map, throw an error
	throw std::runtime_error("Attempted to index a non-container variable: " + variableName);
}
