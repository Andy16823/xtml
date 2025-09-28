#include "ModuleStd.h"
#include <algorithm>
#include <cctype>
#include "Utils.h"

using namespace std;


void ModuleStd::RegisterFunctions(FunctionRegistry& registry)
{
	registry.RegisterNamespace("std");
	registry.RegisterFunction("std", "toUpper", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_STRING) {
			Utils::printerr_ln("Error: std::toupper expects a single string argument.");
			return var{ "", DT_UNKNOWN };
		}
		auto result = args[0].value;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		return var{ result, DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "toLower", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_STRING) {
			Utils::printerr_ln("Error: std::tolower expects a single string argument.");
			return var{ "", DT_UNKNOWN };
		}
		auto result = args[0].value;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return var{ result, DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "randStr", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_NUMBER || !Utils::is_number(args[0].value)) {
			Utils::printerr_ln("Error: std::randStr expects a single numeric argument.");
			return var{ "", DT_UNKNOWN };
		}
		int length = std::stoi(args[0].value);
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		std::string result;
		result.resize(length);
		for (int i = 0; i < length; ++i) {
			result[i] = charset[rand() % max_index];
		}
		return var{ result, DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "toInt", [](const vector<var>& args) -> var {
		if (args.size() != 1 || !Utils::is_number(args[0].value)) {
			Utils::printerr_ln("Error: std::toInt expects a single string numeric argument");
			return var{ "", DT_UNKNOWN };
		}
		return var{ args[0].value, DT_NUMBER };
		}, 1, 1);

	registry.RegisterFunction("std", "toStr", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_NUMBER) {
			Utils::printerr_ln("Error: std::toStr expects a single numeric argument.");
			return var{ "", DT_UNKNOWN };
		}
		return var{ args[0].value, DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "len", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_STRING) {
			Utils::printerr_ln("Error: std::len expects a single string argument.");
			return var{ "", DT_UNKNOWN };
		}
		return var{ std::to_string(args[0].value.length()), DT_NUMBER };
		}, 1, 1);

	registry.RegisterFunction("std", "trim", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_STRING) {
			Utils::printerr_ln("Error: std::trim expects a single string argument.");
			return var{ "", DT_UNKNOWN };
		}
		return var{ Utils::trim(args[0].value), DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "trimQuotes", [](const vector<var>& args) -> var {
		if (args.size() != 1 || args[0].type != DT_STRING) {
			Utils::printerr_ln("Error: std::trimQuotes expects a single string argument.");
			return var{ "", DT_UNKNOWN };
		}
		return var{ Utils::trim_quotes(args[0].value), DT_STRING };
		}, 1, 1);

	registry.RegisterFunction("std", "uuid", [](const vector<var>& args) -> var {
		if (!args.empty()) {
			Utils::printerr_ln("Error: std::uuid expects no arguments.");
			return var{ "", DT_UNKNOWN };
		}
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		std::string result;
		result.resize(36);
		for (int i = 0; i < 36; ++i) {
			if (i == 8 || i == 13 || i == 18 || i == 23) {
				result[i] = '-';
			}
			else {
				result[i] = charset[rand() % max_index];
			}
		}
		return var{ result, DT_STRING };
		}, 0, 0);
}
