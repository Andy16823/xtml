#pragma once
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "Vars.h"

struct XtmlFunction {
	std::function<var(const std::vector<var>&)> callback;
	size_t minArgs;
	size_t maxArgs;
};

struct XtmlNamespace {
	std::string name;
	std::map<std::string, XtmlFunction> functions;
};

class FunctionRegistry
{
private:
	std::map<std::string, XtmlNamespace> m_namespaces;
public:
	XtmlNamespace RegisterNamespace(const std::string& name);
	bool RegisterFunction(const std::string& namespaceName, const std::string& functionName, std::function<var(const std::vector<var>&)> callback, size_t minArgs = 0, size_t maxArgs = 0);
	var CallFunction(const std::string& namespaceName, const std::string& functionName, const std::vector<var>& args);
	bool Exists(const std::string& namespaceName, const std::string& functionName);

	static std::tuple<std::string, std::string, std::vector<std::string>> ParseFunctionCall(const std::string& expr);

};

