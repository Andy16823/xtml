#pragma once
#include "ASTNode.h"

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
/// Comment Node
/// </summary>
class CommentNode : public StmtNode {
public:
	std::string content;
	EvalResult evaluate(Program& program) override;
};
