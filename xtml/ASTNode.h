#pragma once
#include <string>
#include <map>
#include "Vars.h"
#include <memory>
#include <vector>
#include "Statements.h"

/// <summary>
/// Default AST Node
/// </summary>
class ASTNode
{
public:
	std::vector<std::unique_ptr<ASTNode>> children;
	virtual ~ASTNode() = default;
	virtual std::string evaluate(std::map<std::string, var>& vars) = 0;

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
	std::string evaluate();

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
/// Block Node
/// </summary>
class BlockNode : public ASTNode
{
public:
	std::string evaluate(std::map<std::string, var>& vars) override;
};


/// <summary>
/// Variable Declaration Node
/// </summary>
class VarDeclNode : public ASTNode
{
private:
	std::string m_name;
	std::string m_expr;
public:
	VarDeclNode(const std::string& name, const std::string& expr) : m_name(name), m_expr(expr) {}
	std::string evaluate(std::map<std::string, var>& vars) override;
};

class IfStatementNode : public ASTNode
{
private:
	IfStatement m_if_stmt;
	std::map<std::string, std::vector<std::unique_ptr<ASTNode>>> m_branch_nodes;
public:
	IfStatementNode(const IfStatement& if_stmt) : m_if_stmt(if_stmt) {}
	std::string evaluate(std::map<std::string, var>& vars) override;
	void parse_braches();
};

class TextNode : public ASTNode
{	
private:
	std::string m_value;
public:
	TextNode(const std::string& value) : m_value(value) {}
	std::string evaluate(std::map<std::string, var>& vars) override;
};