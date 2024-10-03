#include "Parser.hpp"
#include "Types.hpp"
#include "Environment.hpp"
#include "Lexer.hpp"

int test1() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});

	// Example script with for loop and do-while loop
	std::string input = R"(
        int x = 8;
        int y;
        y = x - 3;
        if (y > 0) {
            print(y);
        }
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test2() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});

	// Example script with for loop and do-while loop
	std::string input = R"(
        int x = 5;
		if (x > 3) { x = x + 1; }
		while (x < 10) { x = x + 1;  print(x); }
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test3() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});

	// Example script with for loop and do-while loop
	std::string input = R"(
       int x = 5;
        float y = 2.5;
        bool isTrue = true;
        string z = "hi from script";

        print(x);
        print(y);
        print(isTrue);
        print(z);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test4() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});

	// Example script with for loop and do-while loop
	std::string input = R"(
       func int add(int a, int b) {
			return a + b;
		}

		int x = add(5, 10);  // Correct usage
		print(x);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test5() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});
	//TODO fix this
	// Example script with for loop and do-while loop
	std::string input = R"(
func int sumUpTo(int n) {
    int sum = 0;
    int i = 1;

    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }

    return sum;
}

func int factorial(int n) {
    int result = 1;

    for (int i = 1; i <= n; i = i + 1) {
        result = result * i;
    }

    return result;
}

func int incrementTest(int n) {
    int i = 0;

    // Prefix increment
    ++i;

    // Postfix increment
    i++;

    // Using while loop with increment
    while (i < n) {
        i++;
    }

    return i;
}

int main() {
    int result1 = sumUpTo(5);      // Should return 15
    int result2 = factorial(5);    // Should return 120
 int result3 = incrementTest(5);    // Should return 120

    // Assume a print function exists
    print(result1);
    print(result2);
print(result3);
}
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test6() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});
	//TODO fix this
	// Example script with for loop and do-while loop
	std::string input = R"(
func bool evaluateLogic(bool a, bool b) {
    return a && b;  // Logical AND
}

func bool evaluateComplexLogic(bool a, bool b, bool c) {
    return (a && b) || c;  // Logical AND followed by OR
}

func bool testLogic() {
    bool a = true;
    bool b = false;
    bool c = true;

    bool result1 = evaluateLogic(a, b);        // Should return false (true && false)
    bool result2 = evaluateComplexLogic(a, b, c); // Should return true ((true && false) || true)

    return result1 || result2;  // This should return true
}

int main() {
    bool result1 = testLogic();      // Should return 15

    // Assume a print function exists
    print(result1);
}
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}


int test7() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});
	//TODO fix this
	// Example script with for loop and do-while loop
	std::string input = R"(
		int i;
        for (i = 0; i < 5; i = i + 1) {
            print(i);
        }

        int x = 0;
        do {
            print(x);
            x = x + 1;
        } while (x < 3);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}


int test8() {
	Environment env;

	// Example script with for loop and do-while loop, which also includes a function definition.
	std::string input = R"(
        func int multiply(int a, int b) {
            return a * b;
        }
    )";

	// Create Lexer and Parser
	Lexer lexer(input);
	Parser parser(lexer, env);

	// Parse the script and register the function
	ASTNode* root{ nullptr };
	try {
		root = parser.parse();
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}

	//run the program
	try {
		root->evaluate(env);  // This will register the function "multiply" in the environment
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}
	

	// Now that the function is defined in the environment, call it from C++
	try {
		// Arguments to pass to the function
		std::vector<double> args = { 5, 10 };

		// Evaluate the function in the script environment
		double result = env.evaluateFunction("multiply", args);

		std::cout << "Result of multiply(5, 10): " << result << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error during function call: " << e.what() << std::endl;
	}

	delete root;
	return 0;
}

double addNumbers(const std::vector<double>& args) {
	if (args.size() != 2) {
		throw std::runtime_error("addNumbers expects exactly 2 arguments.");
	}
	return args[0] + args[1];
}

int test9() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", [](const std::vector<double>& args) -> double {
		if (args.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}
		std::cout << "Print from script: " << args[0] << std::endl;
		return 0;
		});

	// Register the C++ function addNumbers into the environment
	env.registerFunction("addNumbers", addNumbers);

	// Example script with for loop and do-while loop, which also includes a function definition.
	std::string input = R"(
         int result = addNumbers(7, 8);
		 print(result);
    )";

	// Create Lexer and Parser
	Lexer lexer(input);
	Parser parser(lexer, env);

	// Parse the script and register the function
	ASTNode* root{ nullptr };
	try {
		root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}

	
	return 0;
}


int main() {
	
  	test1();
  	test2();
 	test3();
	test4();
	/**test5();
	test6();
	test7();*/
	test8();
	test9();
}

