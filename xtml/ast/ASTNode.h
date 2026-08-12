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

// Grouped by category; included here so existing `#include "ast/ASTNode.h"` keeps working.
#include "ExpressionNodes.h"
#include "ControlFlowNodes.h"
#include "HtmlNodes.h"
