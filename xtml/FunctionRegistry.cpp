#include "FunctionRegistry.h"
#include "Utils.h"

using namespace std;

XtmlNamespace FunctionRegistry::RegisterNamespace(const std::string& name)
{
	auto ns = XtmlNamespace{ name, {} };
	m_namespaces[name] = ns;
	return ns;
}

bool FunctionRegistry::RegisterFunction(const std::string& namespaceName, const std::string& functionName, std::function<var(const std::vector<var>&)> callback, size_t minArgs, size_t maxArgs)
{
	auto it = m_namespaces.find(namespaceName);
	if (it != m_namespaces.end()) {
		it->second.functions[functionName] = XtmlFunction{ callback, minArgs, maxArgs };
		return true;
	}
	return false;
}

var FunctionRegistry::CallFunction(const std::string& namespaceName, const std::string& functionName, const std::vector<var>& args)
{
	auto nsIt = m_namespaces.find(namespaceName);
	if (nsIt != m_namespaces.end()) {
		auto funcIt = nsIt->second.functions.find(functionName);
		if (funcIt != nsIt->second.functions.end()) {
			auto& func = funcIt->second;
			if ((func.minArgs == 0 && func.maxArgs == 0) || (args.size() >= func.minArgs && (func.maxArgs == 0 || args.size() <= func.maxArgs))) {
				return func.callback(args);
			}
			else {
				Utils::printerr_ln("Error: Function " + namespaceName + "::" + functionName + " called with invalid number of arguments.");
				return var();
			}
		}
		else {
			Utils::printerr_ln("Error: Function " + namespaceName + "::" + functionName + " not found.");
			return var();
		}
	}
	return var();
}

bool FunctionRegistry::Exists(const std::string& namespaceName, const std::string& functionName)
{
	auto nsIt = m_namespaces.find(namespaceName);
	if (nsIt != m_namespaces.end()) {
		auto funcIt = nsIt->second.functions.find(functionName);
		if (funcIt != nsIt->second.functions.end()) {
			return true;
		}
	}
	return false;
}

tuple<std::string, std::string, vector<string>> FunctionRegistry::ParseFunctionCall(const std::string& expr)
{
	std::vector<string> args;
	// Parse a function call expression e.g. namespace::funcName(arg1, arg2)
	auto nsFuncSplit = expr.find("::");
	if (nsFuncSplit == std::string::npos) {
		Utils::throw_err("Error: Invalid function call expression: " + expr);
	}
	auto namespaceName = Utils::trim(expr.substr(0, nsFuncSplit));
	auto rest = Utils::trim(expr.substr(nsFuncSplit + 2));
	auto parenPos = rest.find('(');
	if (parenPos == std::string::npos || rest.back() != ')') {
		Utils::throw_err("Error: Invalid function call expression: " + expr);
	}
	auto functionName = Utils::trim(rest.substr(0, parenPos));

	auto argsStr = Utils::trim(rest.substr(parenPos + 1, rest.size() - parenPos - 2));
	args = parse_function_args(argsStr);

	return make_tuple(namespaceName, functionName, args);
}

std::vector<std::string> FunctionRegistry::parse_function_args(const std::string& argsStr)
{
	vector<string> args;
	string current;
	size_t paren_depth = 0;
	bool in_quotes = false;
	// math::add(10, 100), math::add(5, 10)
	for (size_t i = 0; i < argsStr.size(); ++i) {
		char c = argsStr[i];

		if (c == '"' && (i == 0 || argsStr[i - 1] != '\\')) {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}

		if (!in_quotes) {
			if (c == '(') paren_depth++;
			else if (c == ')') paren_depth--;
			else if (c == ',' && paren_depth == 0) {
				args.push_back(Utils::trim(current));
				current.clear();
				continue;
			}
		}
		current += c;
	}

	if (!current.empty()) {
		args.push_back(Utils::trim(current));
	}
	return args;
}
