#pragma once
#include "ASTNode.h"

/// <summary>
/// Function Call Node
/// </summary>
class FunctionCallNode : public ExprNode {
	
public:
	std::string functionName;
	std::vector<std::unique_ptr<ExprNode>> arguments;
	FunctionCallNode() = default;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Binary Expression Node
/// </summary>
class BinaryExprNode : public ExprNode
{
public:
	std::unique_ptr<ExprNode> left;
	std::unique_ptr<ExprNode> right;
	std::string op; // Operator
	EvalResult evaluate(Program& program) override;
};

class UnaryExprNode : public ExprNode
{
	public:
	std::unique_ptr<ExprNode> operand;
	std::string op; 
	bool isPrefix;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Expression Statement Node
/// </summary>
class ExprStatementNode : public StmtNode
{
	public:
	std::unique_ptr<ExprNode> expression;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Integer Literal Node
/// </summary>
class IntegerLiteralNode : public ExprNode
{
	public:
	int64_t value;
	IntegerLiteralNode(int64_t val) : value(val) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// String Literal Node
/// </summary>
class StringLiteralNode : public ExprNode
{
	public:
	std::string value;
	StringLiteralNode(const std::string& val) : value(val) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Float Literal Node
/// </summary>
class FloatLiteralNode : public ExprNode
{
	public:
	double value;
	FloatLiteralNode(double val) : value(val) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Boolean Literal Node
/// </summary>
class BoolLiteralNode : public ExprNode
{
	public:
	bool value;
	BoolLiteralNode(bool val) : value(val) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Double Literal Node
/// </summary>
class DoubleLiteralNode : public ExprNode
{
	public:
	double value;
	DoubleLiteralNode(double val) : value(val) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Array Literal Node
/// </summary>
class ArrayLiteralNode : public ExprNode {
public:
	ArrayLiteralNode() = default;
	std::vector<std::unique_ptr<ExprNode>> elements;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Variable Expression Node
/// </summary>
class VarExprNode : public ExprNode
{
	public:
	std::string name;
	VarExprNode(const std::string& varName) : name(varName) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Native Function Call Node
/// e.g. std::print("Hello World");
/// </summary>
class NativeFunctionCallNode : public ExprNode {
public:
	std::string namespaceName;
	std::string functionName;
	std::vector<std::unique_ptr<ExprNode>> arguments;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Node that represents a statement used as an expression
/// Note: need to be careful with this as not all statements can be expressions
/// also need to be explicit market with expr beforehand e.g. return expr html { ... };
/// </summary>
class StmtExprNode : public ExprNode {
public:
	std::unique_ptr<StmtNode> statement;
	StmtExprNode() = default;
	EvalResult evaluate(Program& program) override;
};
