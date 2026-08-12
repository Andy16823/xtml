#pragma once
#include <string>
#include <map>
#include "Vars.h"
#include <memory>
#include <vector>
#include "Statements.h"
#include <stdint.h>
#include "Core.h"


/// <summary>
/// Evaluation Result
/// </summary>
struct EvalResult {
	var evaluated; // The evaluated content TODO: rename to value?
	bool should_break = false; // For loop control
	bool should_continue = false; // For loop control
	bool should_return = false; // For function return control
	std::string output; // Captured printed output
};


/// <summary>
/// Default AST Node
/// </summary>
class ASTNode
{
protected:
	virtual EvalResult merge_results(const EvalResult& a, const EvalResult& b, bool mergeContent = false);

public:
	std::vector<std::unique_ptr<ASTNode>> children;
	virtual ~ASTNode() = default;
	virtual EvalResult evaluate(Program& program) = 0;

	void add_child(std::unique_ptr<ASTNode> child) {
		children.push_back(std::move(child));
	}
};

/// <summary>
/// Root Node
/// </summary>
class RootNode : public ASTNode {
public:
	Program program;
	std::vector<std::unique_ptr<ASTNode>> nodes;
	EvalResult evaluate();
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Statement Node
/// </summary>
class StmtNode : public ASTNode
{
	
public:
	EvalResult evaluate(Program& program);
};

/// <summary>
/// Expression Node
/// </summary>
class ExprNode : public ASTNode
{
	
public:
	EvalResult evaluate(Program& program);
};

/// <summary>
/// Block Node
/// </summary>
class BlockNode : public ASTNode
{
public:
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Xtml Block Node
/// </summary>
class XtmlBlockNode : public ASTNode {
public:
	Program localProgram;
	std::unique_ptr<BlockNode> body;

	EvalResult evaluate(Program& program) override;
};

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
/// Text Node
/// </summary>
class TextNode : public ASTNode
{
private:
	std::string m_value;
public:
	TextNode(const std::string& value) : m_value(value) {}
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

/// <summary>
/// Html Block Node
/// Holds a block of HTML statements
/// Note: returns the html within the evaluation value not output
/// also deeply evaluates its children and merges there result values
/// </summary>
class HtmlBlockNode : public ASTNode {
	
public:
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Root HTML Statement Node
/// Note: wraps an HTML block
/// e.g. html { ... }
/// </summary>
class HtmlStmtRootNode : public StmtNode {
	public:
		std::unique_ptr<HtmlBlockNode> body;
		EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Html Statement Node
/// An HTML tag with attributes and children
/// Note: Returns the html within the evaluation value not output
/// since its the evaluated content
/// </summary>
class HtmlStmtNode : public StmtNode {
public:
	std::string tagName;
	std::map<std::string, std::string> attributes;
	bool selfClosing = false;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Plain HTML Text Node
/// Note: must be an string literal
/// </summary>
class HtmlTextNode : public StmtNode {
public:
	HtmlTextNode() = default;
	HtmlTextNode(const std::string& text) : content(text) {}
	std::string content;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Include Node
/// Includes another XTML file and evaluates its content
/// </summary>
class IncludeNode : public StmtNode {
public:
	std::string includePath;
	std::unique_ptr<RootNode> includedRoot;
	EvalResult evaluate(Program& program) override;
};

/// <summary>
/// Print Node
/// Note: prints the eval result value of the expression to the output
/// </summary>
class PrintNode : public StmtNode {
public:
	std::unique_ptr<ExprNode> expression;
	PrintNode() = default;
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
/// Comment Node
/// </summary>
class CommentNode : public StmtNode {
public:
	std::string content;
	EvalResult evaluate(Program& program) override;
};
