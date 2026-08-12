#include "ExpressionNodes.h"
#include "Utils.h"
#include "Vars.h"
#include "Globals.h"

EvalResult BinaryExprNode::evaluate(Program& program)
{
	EvalResult result;

	auto optype = Vars::getBinaryOpType(op);
	// Early check for unknown operation
	if (optype == BinaryOpType::Unknown) {
		Utils::throwErr("Error: Unknown binary operation: " + op);
	}

	if (optype == BinaryOpType::Assignment) {
		auto leftVar = dynamic_cast<VarExprNode*>(left.get());
		if (leftVar == nullptr) {
			Utils::throwErr("Error: Left side of assignment must be a variable.");
		}
		auto rightResult = right->evaluate(program);
		if(program.vars.find(leftVar->name) == program.vars.end()) {
			Utils::throwErr("Error: Undefined variable: " + leftVar->name);
		}
		program.vars[leftVar->name] = rightResult.evaluated;
		return rightResult; 
	}

	// Logic operation
	if (optype == BinaryOpType::Logical) {
		if (op == "&&") {
			auto leftresult = left->evaluate(program);
			// Short-circuit evaluation
			if (leftresult.evaluated.value == "false") {
				result.evaluated = {"false", DT_BOOL};
				return result;
			}
			auto rightresult = right->evaluate(program);
			if (rightresult.evaluated.value == "false") {
				result.evaluated = {"false", DT_BOOL};
				return result;
			}
			result.evaluated = { "true", DT_BOOL };
			return result;
		}
		if (op == "||") {
			auto leftresult = left->evaluate(program);
			// Short-circuit evaluation
			if (leftresult.evaluated.value == "true") {
				result.evaluated = {"true", DT_BOOL};
				return result;
			}
			auto rightresult = right->evaluate(program);
			if (rightresult.evaluated.value == "true") {
				result.evaluated = {"true", DT_BOOL};
				return result;
			}
			result.evaluated = {"false", DT_BOOL};
			return result;
		}
	}

	// Evaluate left and right expressions
	auto leftResult = left->evaluate(program);
	auto rightResult = right->evaluate(program);

	if (optype == BinaryOpType::Comparison) {
		bool cmpResult = Vars::compareOperation(leftResult.evaluated, rightResult.evaluated, op);
		if (cmpResult) {
			result.evaluated = {"true", DT_BOOL};
		}
		else {
			result.evaluated = { "false", DT_BOOL };
		}
		return result;
	}	

	// Return result
	result.evaluated = Vars::binaryOperation(leftResult.evaluated, rightResult.evaluated, op);
	return result;
}

EvalResult UnaryExprNode::evaluate(Program& program)
{
	// Check that operand is a variable
	if (auto varexpr = dynamic_cast<VarExprNode*>(operand.get())) {
		auto operandResult = varexpr->evaluate(program);

		// Check that variable exists
		if (program.vars.find(varexpr->name) == program.vars.end()) {
			Utils::throwErr("Error: Undefined variable: " + varexpr->name);
		}
		auto varRef = program.vars[varexpr->name];

		// Perform unary operation and update variable
		auto value = Vars::unaryOperation(varRef, op);
		program.vars[varexpr->name] = value;

		// Return the new value
		EvalResult result;

		if (this->isPrefix) {
			result.evaluated = value;
		}
		else {
			result.evaluated = operandResult.evaluated;
		}
		return result;
	}
	
	// Check that operand is a literal
	if (auto intlit = dynamic_cast<IntegerLiteralNode*>(operand.get())) {
		var varRef = { std::to_string(intlit->value), DT_NUMBER };
		auto value = Vars::unaryOperation(varRef, op);
		EvalResult result;
		result.evaluated = value;
		return result;
	}

	Utils::throwErr("Error: Unary operator applied to unsupported operand type.");
}

EvalResult ExprStatementNode::evaluate(Program& program)
{
	return this->expression->evaluate(program);
}

EvalResult IntegerLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.evaluated = {std::to_string(value), DT_NUMBER};
	return result;
}

EvalResult StringLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.evaluated = {value, DT_STRING};
	return result;
}

EvalResult FloatLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.evaluated = {std::to_string(value), DT_NUMBER};
	return result;
}

EvalResult BoolLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	if (value) {
		result.evaluated = { "true", DT_BOOL };
	}
	else {
		result.evaluated = { "false", DT_BOOL };
	}
	return result;
}

EvalResult DoubleLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.evaluated = { std::to_string(value), DT_NUMBER };
	return result;
}

EvalResult ArrayLiteralNode::evaluate(Program& program)
{
	DataType datatype = DT_UNKNOWN;	// Type for the array elements
	std::vector<var> arrayElements;
	for (auto& element : elements) {
		auto elementResult = element->evaluate(program);
		if (datatype == DataType::DT_UNKNOWN) {
			datatype = elementResult.evaluated.type;
		}
		if(elementResult.evaluated.type != datatype) {
			Utils::throwErr("Error: Array elements must be of the same type.");
		}
		arrayElements.push_back(elementResult.evaluated);
	}
	return { "", DT_ARRAY, arrayElements };
}

EvalResult VarExprNode::evaluate(Program& program)
{
	EvalResult result;
	if(program.vars.find(this->name) != program.vars.end()) {
		result.evaluated = program.vars[this->name];
	}
	else {
		Utils::throwErr("Error: Undefined variable: " + this->name);
	}
	return result;
}

EvalResult FunctionCallNode::evaluate(Program& program)
{
	EvalResult result;
	if (program.functions.find(this->functionName) == program.functions.end()) {
		Utils::throwErr("Error: Undefined function: " + this->functionName);
	}
	auto function = program.functions[this->functionName].function;

	std::vector<std::string> argValues;
	for (auto& arg : arguments) {
		auto argResult = arg->evaluate(program);
		argValues.push_back(argResult.evaluated.value);
	}
	return function->callFunction(program, argValues);
}

EvalResult NativeFunctionCallNode::evaluate(Program& program)
{
	// Build arguments
	std::vector<var> args;
	for (auto& arg : arguments) {
		auto argResult = arg->evaluate(program);
		args.push_back(argResult.evaluated);
	}
	
	// Call native function
	var value = g_functionRegistry.CallFunction(this->namespaceName, this->functionName, args);

	// Return result
	EvalResult result;
	result.evaluated = value;
	return result;
}

EvalResult StmtExprNode::evaluate(Program& program)
{
	auto result = statement->evaluate(program);
	result.evaluated = {result.output, DT_STRING};
	result.output = "";
	return result;
}
