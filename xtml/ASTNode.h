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
	struct Branch
	{
		std::string condition;
		std::string content;
		std::vector<std::unique_ptr<ASTNode>> children;
	};

private:
	std::vector<Branch> m_branches;
	bool m_has_else = false;
	Branch m_else_branch;

	void parse_branch(Branch& branch);
public:
	IfStatementNode();
	void add_branch(std::string condition, std::string content);
	void add_else(std::string content);
	bool is_empty() const { return m_branches.empty() && !m_has_else; }

	std::string evaluate(std::map<std::string, var>& vars) override;
};

class TextNode : public ASTNode
{
private:
	std::string m_value;
public:
	TextNode(const std::string& value) : m_value(value) {}
	std::string evaluate(std::map<std::string, var>& vars) override;
};

class WhileNode : public ASTNode
{
private:
	std::string m_condition;
	std::vector<std::unique_ptr<ASTNode>> m_body;
public:
	WhileNode(const std::string& condition, const std::string& body);

	std::string evaluate(std::map<std::string, var>& vars) override;
};

class ForNode : public ASTNode
{
private:
	std::string m_init;
	std::string m_condition;
	std::string m_increment;
	std::vector<std::unique_ptr<ASTNode>> m_body;

	void parse_loop(const std::string& loop_expr, const std::string& body);

public:
	ForNode(const std::string& loop_expr, const std::string& body);
	std::string evaluate(std::map<std::string, var>& vars) override;
};