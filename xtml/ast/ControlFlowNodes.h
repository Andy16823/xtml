#pragma once
#include "ASTNode.h"

/// <summary>
/// Function Node
/// </summary>
class FunctionNode : public StmtNode {

public:
	std::string name;
	std::vector<std::unique_ptr<ExprNode>> arguments;
	std::unique_ptr<BlockNode> body;
	EvalResult evaluate(Program& program) override;
	EvalResult callFunction(Program& program, const std::vector<std::string>& argValues);
};

/// <summary>
/// Function Declaration Node
/// </summary>
class FunctionDeclNode : public StmtNode {

public:
	std::unique_ptr<FunctionNode> function;
	FunctionDeclNode() = default;
	FunctionDeclNode(std::unique_ptr<FunctionNode> func) : function(std::move(func)) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Return Node for functions
/// </summary>
class ReturnNode : public StmtNode {
public:
	std::unique_ptr<ExprNode> expression;
	ReturnNode() = default;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Variable Declaration Node
/// </summary>
class VarDeclNode : public StmtNode
{
public:
	std::string name;
	std::unique_ptr<ExprNode> expression;

	VarDeclNode() = default;
	VarDeclNode(const std::string& name, std::unique_ptr<ExprNode> expr) : name(name), expression(std::move(expr)) {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// If Statement Node
/// </summary>
class IfStatementNode : public StmtNode
{
public:
	std::unique_ptr<ExprNode> condition;
	std::unique_ptr<BlockNode> body;
	std::unique_ptr<BlockNode> elseBody;
	std::vector<std::unique_ptr<IfStatementNode>> elseIfs;

	IfStatementNode();
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// While Node
/// </summary>
class WhileNode : public StmtNode
{
public:
	std::unique_ptr<ExprNode> condition;
	std::unique_ptr<BlockNode> body;

	WhileNode() = default;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// For Node
/// </summary>
class ForNode : public StmtNode
{
public:
	std::unique_ptr<StmtNode> init;
	std::unique_ptr<ExprNode> condition;
	std::unique_ptr<ExprNode> increment;
	std::unique_ptr<BlockNode> body;
	ForNode() = default;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// For Each Node
/// Not implemented yet
/// </summary>
class ForEachNode : public ASTNode {
public:
	std::unique_ptr<StmtNode> declaration;
	std::unique_ptr<BlockNode> body;
	ForEachNode(const std::string& expression, const std::string& body);
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Break Node for loops
/// </summary>
class BreakNode : public StmtNode
{
public:
	BreakNode() {}
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Continue Node for loops
/// </summary>
class ContinueNode : public StmtNode
{
public:
	ContinueNode() {}
	EvalResult evaluate(Program& program) override;
};
