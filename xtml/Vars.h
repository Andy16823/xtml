#pragma once  
#include <string>  
#include <map>  
#include <vector>  

/// <summary>
/// Data Types
/// </summary>
enum DataType
{
	DT_STRING,
	DT_NUMBER,
	DT_BOOL,
	DT_ARRAY,
	DT_UNKNOWN
};

/// <summary>
/// Binary Operation Types
/// </summary>
enum class BinaryOpType { 
	Assignment, 
	Comparison, 
	Arithmetic, 
	Logical,
	Unknown 
};

/// <summary>
/// Variable Structure
/// </summary>
struct var {
	std::string value;
	DataType type;
	std::vector<var> array; // For DT_ARRAY type
};

/// <summary>
/// Overloaded + operator for variables
/// </summary>
/// <param name="lhs"></param>
/// <param name="rhs"></param>
/// <returns></returns>
var operator+(const var& lhs, const var& rhs);

/// <summary>
/// Variable Utilities
/// </summary>
class Vars  
{  
public: 	
	/// <summary>
	/// Determine if the tokens represent a function expression
	/// </summary>
	/// <param name="tokens"></param>
	/// <returns></returns>
	static bool isFunctionExpr(std::vector<std::string>& tokens);

	/// <summary>
	/// Evaluate an expression and return the resulting variable
	/// </summary>
	/// <param name="expr"></param>
	/// <param name="vars"></param>
	/// <returns></returns>
	static var evalExpr(const std::string& expr, const std::map<std::string, var>& vars);

	/// <summary>
	/// Evaluate a function expression from tokens and return the resulting variable
	/// </summary>
	/// <param name="tokens"></param>
	/// <param name="vars"></param>
	/// <returns></returns>
	static var evalFuncExpr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);

	/// <summary>
	/// Determine if the token represents a function expression
	/// </summary>
	/// <param name="token"></param>
	/// <returns></returns>
	static bool isFunctionExpr(const std::string& token);

	/// <summary>
	/// Evaluate a function expression from a token and return the resulting variable
	/// </summary>
	/// <param name="token"></param>
	/// <param name="vars"></param>
	/// <returns></returns>
	static var evalFuncExpr(const std::string& token, const std::map<std::string, var>& vars);

	/// <summary>
	/// Perform a binary operation between two variables
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static var binaryOperation(const var& left, const var& right, const std::string& op);

	/// <summary>
	/// Perform a string operation between two variables
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static var stringOperation(const var& left, const var& right, const std::string& op);

	/// <summary>
	/// Perform a numeric operation between two variables
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static var numericOperation(const var& left, const var& right, const std::string& op);

	/// <summary>
	/// Perform a boolean operation between two variables
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static var booleanOperation(const var& left, const var& right, const std::string& op);

	/// <summary>
	/// Perform a comparison operation between two variables
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static bool compareOperation(const var& left, const var& right, const std::string& op);

	/// <summary>
	/// Compare two strings with the given operator
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static bool compareStrings(const std::string& left, const std::string& right, const std::string& op);

	/// <summary>
	/// Compare two numbers with the given operator
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static bool compareNumbers(int64_t left, int64_t right, const std::string& op);

	/// <summary>
	/// Compare two booleans with the given operator
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static bool compareBooleans(bool left, bool right, const std::string& op);

	/// <summary>
	/// Perform a unary operation on a variable
	/// </summary>
	/// <param name="operand"></param>
	/// <param name="op"></param>
	/// <returns></returns>
	static var unaryOperation(const var& operand, const std::string& op);

	/// <summary>
	/// Get the binary operation type from an operator string
	/// </summary>
	/// <param name="op"></param>
	/// <returns></returns>
	static BinaryOpType getBinaryOpType(const std::string& op);
};
