#include "ASTNode.h"
#include "Utils.h"
#include "Core.h"

EvalResult ASTNode::merge_results(const EvalResult& a, const EvalResult& b, bool mergeContent)
{
	EvalResult result;
	if (mergeContent) {
		result.evaluated = a.evaluated + b.evaluated;
	}
	result.output = a.output + b.output;

	if (a.should_break || b.should_break) {
		result.should_break = true;
	}else if (a.should_continue || b.should_continue) {
		result.should_continue = true;
	}

	if (a.should_return || b.should_return) {
		result.should_return = true;
	}

	return result;
}

EvalResult BlockNode::evaluate(Program& program)
{
	EvalResult result;
	for (auto& child : children) {
		auto childResult = child->evaluate(program);
		// Handle Return
		if (childResult.should_return) {
			return childResult;
		}

		// Handle Break
		if (childResult.should_break) {
			result.should_break = true;
			return result;
		}

		// Handle Continue
		if (childResult.should_continue) {
			result.should_continue = true;
			return result;
		}

		result = merge_results(result, childResult);
	}
	return result;
}

EvalResult StmtNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult ExprNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult XtmlBlockNode::evaluate(Program& program)
{
	EvalResult result;

	// First move global vars to local scope
	Core::mergePrograms(program, localProgram);

	// Evaluate body
	result = merge_results(result, this->body->evaluate(this->localProgram));

	// Merge back local vars to global scope
	Core::mergePrograms(this->localProgram, program);

	// Return result
	return result;
}

EvalResult RootNode::evaluate()
{
	EvalResult result;
	Program& p = this->program;
	for (auto& node : nodes) {
		result = merge_results(result, node->evaluate(p));
	}
	return result;
}

EvalResult RootNode::evaluate(Program& program)
{
	Utils::throwErr("Error: RootNode evaluate with Program parameter is not supported. Use the parameterless evaluate() method instead.");
	return {};
}
