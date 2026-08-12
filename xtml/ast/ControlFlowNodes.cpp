#include "ControlFlowNodes.h"
#include "ExpressionNodes.h"
#include "Utils.h"
#include "Core.h"

EvalResult VarDeclNode::evaluate(Program& program)
{
	// Declare a variable
	if(this->expression == nullptr) {
		var value;
		value.value = "";
		value.type = DT_UNKNOWN;
		return EvalResult{}; // No expression to evaluate
	}

	// Evaluate the expression and assign to variable
	EvalResult eval = expression->evaluate(program);
	program.vars[name] = eval.evaluated;

	// No output for variable declaration
	return {};
}

IfStatementNode::IfStatementNode()
{
	
}

EvalResult IfStatementNode::evaluate(Program& program)
{
	EvalResult result;

	// Try if condition
	auto condResult = condition->evaluate(program);
	if(condResult.evaluated.value == "true") {
		result = body->evaluate(program);
		return result;
	}
	
	// Try else ifs
	for (auto& elseif : elseIfs) {
		auto elseIfresult = elseif->condition->evaluate(program);
		if (elseIfresult.evaluated.value == "true") {
			result = elseif->body->evaluate(program);
			return result; // Return because else if was executed
		}
	}

	// Try else body
	if(elseBody != nullptr) {
		result = elseBody->evaluate(program);
	}
	return result;
}

EvalResult WhileNode::evaluate(Program& program)
{
	EvalResult result;

	auto condResult = condition->evaluate(program);
	auto cond = Utils::toBool(condResult.evaluated.value);
	while (cond) {
		// Evaluate body
		auto bodyResult = this->body->evaluate(program);

		// Check for break
		if (bodyResult.should_break) {
			bodyResult.should_break = false;
			result = merge_results(result, bodyResult);
			break;
		}

		// Check for continue (skip to next iteration)
		if (bodyResult.should_continue) {
			bodyResult.should_continue = false;
			result = merge_results(result, bodyResult);
			// Re-evaluate condition
			condResult = condition->evaluate(program);
			cond = Utils::toBool(condResult.evaluated.value);
			continue;
		}

		// Normal execution
		result = merge_results(result, bodyResult);
		condResult = condition->evaluate(program);
		cond = Utils::toBool(condResult.evaluated.value);
	}

	return result;
}

EvalResult ForNode::evaluate(Program& program)
{
	EvalResult result;

	// Declare init statement
	if (init != nullptr) {
		init->evaluate(program);	// Initialize loop variable
	}
	else {
		Utils::throwErr("Error: For loop missing initialization statement.");
	}

	// Evaluate condition
	auto condResult = condition->evaluate(program);
	while (condResult.evaluated.value == "true") {
		auto bodyResult = body->evaluate(program);

		// Handle break, we exit the loop
		if (bodyResult.should_break) {
			bodyResult.should_break = false;
			result = merge_results(result, bodyResult);
			break;
		}

		// Handle continue, we skip to the next iteration
		if (bodyResult.should_continue) {
			bodyResult.should_continue = false;
			result = merge_results(result, bodyResult);

			if (increment) increment->evaluate(program);
			else {
				Utils::throwErr("Error: For loop missing increment statement.");
			}
			condResult = condition->evaluate(program);
			continue;
		}

		// Normal execution
		result = merge_results(result, bodyResult);
		if (increment) increment->evaluate(program);
		else {
			Utils::throwErr("Error: For loop missing increment statement.");
		}
		condResult = condition->evaluate(program);
	}

	return result;
}

EvalResult ForEachNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult BreakNode::evaluate(Program& program)
{
	// Note: Break stops execution of the block AND the loop
	EvalResult result;
	result.should_break = true;
	return result;
}

EvalResult ContinueNode::evaluate(Program& program)
{
	// Note: Continue stops execution the block BUT NOT the loop
	EvalResult result;
	result.should_continue = true;
	return result;
}

EvalResult FunctionNode::evaluate(Program& program)
{
	return this->body->evaluate(program);
}

EvalResult FunctionNode::callFunction(Program& program, const std::vector<std::string>& argValues)
{
	if(argValues.size() != this->arguments.size()) {
		Utils::throwErr("Error: Function " + this->name + " called with incorrect number of arguments.");
	}

	// Create a new program frame for the function call
	Program frame = Program();
	Core::mergeProgrammFunctions(program, frame);
	frame.path = program.path;

	// Bind arguments to local program variables
	for (size_t i = 0; i < this->arguments.size(); i++) {
		auto argExpr = dynamic_cast<VarExprNode*>(this->arguments[i].get());
		if (argExpr == nullptr) {
			Utils::throwErr("Error: Function argument must be a variable.");
		}
		var value;
		value.value = argValues[i];
		value.type = Utils::predictVarType(value.value);
		frame.vars[argExpr->name] = value;
	}

	// Evaluate function body in local program context with evaluate from FunctionNode
	auto result = this->evaluate(frame);
	result.should_return = false;
	return result;
}

EvalResult FunctionDeclNode::evaluate(Program& program)
{
	FunctionCall funcCall;
	funcCall.name = function->name;
	funcCall.function = function.get();
	program.functions[function->name] = funcCall;

	return {};
}

EvalResult ReturnNode::evaluate(Program& program)
{
	if(this->expression == nullptr) {
		Utils::throwErr("Error: Return statement missing expression.");
	}

	auto result = this->expression->evaluate(program);
	result.should_return = true;
	return result;
}
