#pragma once
#include <string>
#include <map>
#include "Vars.h"
#include <memory>
#include <vector>
#include "Statements.h"
#include <stdint.h>



struct EvalResult {
	std::string content; // The evaluated content TODO: rename to value?
	bool should_break = false; // For loop control
	bool should_continue = false; // For loop control
	bool print_result = true;	// Whether the result should be printed to output
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

class XtmlBlockNode : public ASTNode {
public:
	std::map<std::string, var> localVars;
	std::unique_ptr<BlockNode> body;

	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Function Node
/// </summary>
class FunctionNode : public ASTNode {
	public:
		std::string name;
		std::vector<std::unique_ptr<ExprNode>> arguments;
		std::unique_ptr<BlockNode> body;
		EvalResult evaluate(std::map<std::string, var>& vars) override;
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
	EvalResult evaluate(std::map<std::string, var>& vars) override;
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
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

class UnaryExprNode : public ExprNode
{
	public:
	std::unique_ptr<ExprNode> operand;
	std::string op; 
	bool isPrefix;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Expression Statement Node
/// </summary>
class ExprStatementNode : public StmtNode
{
	public:
	std::unique_ptr<ExprNode> expression;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Integer Literal Node
/// </summary>
class IntegerLiteralNode : public ExprNode
{
	public:
	int64_t value;
	IntegerLiteralNode(int64_t val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// String Literal Node
/// </summary>
class StringLiteralNode : public ExprNode
{
	public:
	std::string value;
	StringLiteralNode(const std::string& val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Float Literal Node
/// </summary>
class FloatLiteralNode : public ExprNode
{
	public:
	double value;
	FloatLiteralNode(double val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Boolean Literal Node
/// </summary>
class BoolLiteralNode : public ExprNode
{
	public:
	bool value;
	BoolLiteralNode(bool val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Double Literal Node
/// </summary>
class DoubleLiteralNode : public ExprNode
{
	public:
	double value;
	DoubleLiteralNode(double val) : value(val) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Variable Expression Node
/// </summary>
class VarExprNode : public ExprNode
{
	public:
	std::string name;
	VarExprNode(const std::string& varName) : name(varName) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
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
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Text Node
/// </summary>
class TextNode : public ASTNode
{
private:
	std::string m_value;
public:
	TextNode(const std::string& value) : m_value(value) {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// While Node
/// </summary>
class WhileNode : public ASTNode
{
public:
	std::unique_ptr<ExprNode> condition;
	std::vector<std::unique_ptr<BlockNode>> body;

	WhileNode() = default;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
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
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// For Each Node
/// </summary>
class ForEachNode : public ASTNode {
public:
	std::unique_ptr<StmtNode> declaration;
	std::unique_ptr<BlockNode> body;
	ForEachNode(const std::string& expression, const std::string& body);
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Break Node
/// </summary>
class BreakNode : public ASTNode
{
public:
	BreakNode() {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Continue Node
/// </summary>
class ContinueNode : public ASTNode
{
public:
	ContinueNode() {}
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// HTML Statement Root Node
/// </summary>
class HtmlStmtRootNode : public StmtNode {
	public:
		std::unique_ptr<BlockNode> body;
		EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Html Statement Node
/// </summary>
class HtmlStmtNode : public StmtNode {
public:
	std::string tagName;
	std::map<std::string, std::string> attributes;
	bool selfClosing = false;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};

/// <summary>
/// Html Text Node
/// </summary>
class HtmlTextNode : public StmtNode {
public:
	HtmlTextNode() = default;
	std::string content;
	EvalResult evaluate(std::map<std::string, var>& vars) override;
};