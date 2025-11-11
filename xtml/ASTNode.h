#pragma once
#include <string>
#include <map>
#include "Vars.h"
#include <memory>
#include <vector>
#include "Statements.h"
#include <stdint.h>



struct EvalResult {
	std::string content;
	bool should_break = false;
	bool should_continue = false;
};


/// <summary>
/// Default AST Node
/// </summary>
class ASTNode
{
protected:
	virtual EvalResult merge_results(const EvalResult& a, const EvalResult& b);

public:
	std::vector<std::unique_ptr<ASTNode>> children;
	virtual ~ASTNode() = default;
	virtual EvalResult evaluate(std::map<std::string, var>& vars) = 0;

	void add_child(std::unique_ptr<ASTNode> child) {
		children.push_back(move(child));
	}
};

/// <summary>
/// AST Root Node
/// </summary>
class ASTRoot
{
public:
	std::vector<std::unique_ptr<ASTNode>> children;
	std::map<std::string, var> vars;
	std::string built_content;
	EvalResult evaluate();

	void add_child(std::unique_ptr<ASTNode> child) {
		children.push_back(move(child));
	}

	void merge_vars(const std::map<std::string, var>& new_vars) {
		for (const auto& [key, value] : new_vars) {
			vars[key] = value;
		}
	}
};

/// <summary>
/// Statement Node
/// </summary>
class StmtNode : public ASTNode
{
	public:
	EvalResult evaluate(std::map<std::string, var>& vars);
};


/// <summary>
/// Expression Node
/// </summary>
class ExprNode : public ASTNode
{
	public:
	EvalResult evaluate(std::map<std::string, var>& vars);
};

/// <summary>
/// Block Node
/// </summary>
class BlockNode : public ASTNode
{
public:
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class FunctionNode : public ASTNode {
	public:
		std::string name;
		std::vector<std::unique_ptr<ExprNode>> arguments;
		std::unique_ptr<BlockNode> body;
		std::map<std::string, var> localVars;
		EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class VarDeclNode : public StmtNode
{
private:
	std::string m_name;
	std::string m_expr;
public:
	std::string name;
	std::unique_ptr<ExprNode> expression;

	VarDeclNode() = default;
	VarDeclNode(const std::string& name, const std::string& expr) : m_name(name), m_expr(expr) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class BinaryExprNode : public ExprNode
{
public:
	std::unique_ptr<ExprNode> left;
	std::unique_ptr<ExprNode> right;
	std::string op; // Operator
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class ExprStatementNode : public StmtNode
{
	public:
	std::unique_ptr<ExprNode> expression;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class IntegerLiteralNode : public ExprNode
{
	public:
	int64_t value;
	IntegerLiteralNode(int64_t val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class StringLiteralNode : public ExprNode
{
	public:
	std::string value;
	StringLiteralNode(const std::string& val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class FloatLiteralNode : public ExprNode
{
	public:
	double value;
	FloatLiteralNode(double val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class BoolLiteralNode : public ExprNode
{
	public:
	bool value;
	BoolLiteralNode(bool val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class DoubleLiteralNode : public ExprNode
{
	public:
	double value;
	DoubleLiteralNode(double val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class VarExprNode : public ExprNode
{
	public:
	std::string name;
	VarExprNode(const std::string& varName) : name(varName) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class IfStatementNode : public StmtNode
{
public:
	std::unique_ptr<ExprNode> condition;
	std::unique_ptr<BlockNode> body;

	IfStatementNode();
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class TextNode : public ASTNode
{
private:
	std::string m_value;
public:
	TextNode(const std::string& value) : m_value(value) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class WhileNode : public ASTNode
{
public:
	std::unique_ptr<ExprNode> condition;
	std::vector<std::unique_ptr<BlockNode>> body;

	WhileNode() = default;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class ForNode : public ASTNode
{
public:
	std::unique_ptr<StmtNode> init;
	std::unique_ptr<ExprNode> condition;
	std::unique_ptr<StmtNode> increment;
	ForNode() = default;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class ForEachNode : public ASTNode {
public:
	std::unique_ptr<StmtNode> declaration;
	std::unique_ptr<BlockNode> body;
	ForEachNode(const std::string& expression, const std::string& body);
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class BreakNode : public ASTNode
{
public:
	BreakNode() {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class ContinueNode : public ASTNode
{
public:
	ContinueNode() {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};