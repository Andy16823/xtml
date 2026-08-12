#include "Utils.h"  
#include <algorithm>  
#include <iostream>  
#include <fstream>
#include <sstream> 
#include <chrono>
#include <random>

using namespace std;

bool Utils::isNumber(const std::string& s)
{
	try {
		size_t pos;
		std::stoi(s, &pos);

		// pos == L�nge ? gesamte Zeichenkette ist eine Zahl
		return pos == s.length();
	}
	catch (...) {
		return false;
	}
}

bool Utils::isAlpha(const std::string& s)
{
	return !s.empty() && std::all_of(s.begin(), s.end(), ::isalpha);
}

bool Utils::isString(const std::string& s)
{
	if (s.size() < 2 || s.front() != '"') return false;

	// Pr�fe, ob das letzte Zeichen ein " ist, das nicht escaped ist
	size_t backIndex = s.size() - 1;
	size_t escapeCount = 0;
	while (backIndex > 0 && s[backIndex - 1 - escapeCount] == '\\') {
		escapeCount++;
	}

	return s.back() == '"' && (escapeCount % 2 == 0);
}

bool Utils::isBool(const std::string& s)
{
	if (s == "true" || s == "false" || s == "1" || s == "0") {
		return true;
	}
	return false;
}

void Utils::printLn(const std::string& str)
{
	std::cout << str << std::endl;
}

void Utils::printerrLn(const std::string& str)
{
	std::cerr << str << std::endl;
}

[[noreturn]] void Utils::throwErr(const std::string& str, const std::string& stack_trace)
{	
	const std::string red = "\033[31m";
	const std::string reset = "\033[0m";

	std::cerr << red << "Error: " << str << reset << std::endl;

	if (!stack_trace.empty()) {
		std::cerr << red << "Stack trace:" << reset << std::endl;
		std::cerr << stack_trace << std::endl;
	}

	throw std::runtime_error(str);
}

std::string Utils::escapeStr(const std::string& str)
{
	std::string result = str;
	result = Utils::replace(result, "\\n", "\n");
	result = Utils::replace(result, "\\t", "\t");
	result = Utils::replace(result, "\\\"", "\"");
	result = Utils::replace(result, "\\\\", "\\");
	return result;
}

std::string Utils::fileName(const std::string& file_path)
{
	size_t last_slash = file_path.find_last_of("/\\");
	if (last_slash == std::string::npos) return file_path;
	return file_path.substr(last_slash + 1);
}

std::string Utils::fileNameNoExt(const std::string& file_name)
{
	size_t last_dot = file_name.find_last_of('.');
	if (last_dot == std::string::npos) return file_name;
	return file_name.substr(0, last_dot);
}

std::string Utils::filePathParent(const std::string& file_path)
{
	size_t last_slash = file_path.find_last_of("/\\");
	if (last_slash != std::string::npos)
		return file_path.substr(0, last_slash);
	return "";
}

std::string Utils::trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t\n\r");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, (last - first + 1));
}

std::string Utils::trimQuotes(const std::string& str)
{
	if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
		return str.substr(1, str.size() - 2);
	}
	return str;
}

std::string Utils::readFile(const std::string& filename)
{
	std::ifstream file(filename); // Ensure std::ifstream is included  
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string Utils::replaceWhitespace(const std::string& str, char replacement)
{
	std::string result = str;
	std::replace_if(result.begin(), result.end(), ::isspace, replacement);
	return result;
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

std::string Utils::replace(const std::string& str, const std::string& from, const std::string& to)
{
	std::string result = str;
	size_t start_pos;
	while ((start_pos = result.find(from)) != std::string::npos) {
		result = result.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Move past the replacement
	}
	return result;
}

bool Utils::startsWith(const std::string& str, const std::string& prefix)
{
	if (str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0) {
		return true;
	}
	return false;
}

bool Utils::endsWith(const std::string& str, const std::string& suffix)
{
	if (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
		return true;
	}
	return false;
}

bool Utils::isAbsolute(const std::string& path)
{
	if (path.size() >= 2 && std::isalpha(path[0]) && path[1] == ':') {
		return true; // Windows absolute path (e.g., C:\)
	}
	if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
		return true; // Unix absolute path (e.g., /usr/bin)
	}
	return false;
}

std::string Utils::generateUuid()
{
	int seed = std::chrono::steady_clock::now().time_since_epoch().count();
	static std::mt19937 rng(seed);
	std::uniform_int_distribution<int> dist(0, 61);

	const char charset[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string result(36, ' ');
	for (int i = 0; i < 36; ++i) {
		if (i == 8 || i == 13 || i == 18 || i == 23)
			result[i] = '-';
		else
			result[i] = charset[dist(rng)];
	}

	return result;
}

DataType Utils::predictVarType(const std::string& value)
{
	// TODO: Improve type prediction (e.g., arrays, etc.)
	if (Utils::isNumber(value)) {
		return DT_NUMBER;
	}
	else if (Utils::isBool(value)) {
		return DT_BOOL;
	}
	return DT_STRING; // Default to string
}

bool Utils::toBool(const std::string& value)
{
	if (value == "true" || value == "1") {
		return true;
	}
	return false;
}

std::string Utils::parseParantheses(const std::string& str)
{
	string current;
	bool in_quotes = false;
	int paren_depth = 0;

	for (char c : str) {
		if (c == '"' || c == '\'') {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}

		if (!in_quotes) {
			if (c == '(') {
				paren_depth++;
				if (paren_depth == 1) continue; // Skip the opening parenthesis
			}
			else if (c == ')') {
				paren_depth--;
				if (paren_depth == 0) break; // Stop at the closing parenthesis
			}
		}

		if (paren_depth > 0) {
			current.push_back(c);
		}

	}

	return Utils::trim(current);
}
