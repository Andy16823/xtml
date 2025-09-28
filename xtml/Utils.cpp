#pragma once
#include "Utils.h"  
#include <algorithm>  
#include <iostream>  
#include <fstream>
#include <sstream>  

bool Utils::is_number(const std::string& s)
{  
   return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);  
}  

bool Utils::is_alpha(const std::string& s)
{  
   return !s.empty() && std::all_of(s.begin(), s.end(), ::isalpha);  
}  

bool Utils::is_string(const std::string& s)  
{  
	return !s.empty() && s.front() == '"' && s.back() == '"';  
}  

void Utils::print_ln(const std::string& str)  
{  
   std::cout << str << std::endl;  
}  

void Utils::printerr_ln(const std::string& str)  
{  
   std::cerr << str << std::endl;  
}

void Utils::throw_err(const std::string& str, const std::string& stack_trace)
{
	std::cerr << "Error: " << str << std::endl;
	if (!stack_trace.empty()) {
		std::cerr << "Stack trace:" << std::endl;
		std::cerr << stack_trace << std::endl;
	}
	throw std::runtime_error(str);
}

std::string Utils::escape_str(const std::string& str)
{
	std::string result = str;
	result = Utils::replace(result, "\\n", "\n");
	result = Utils::replace(result, "\\t", "\t");
	result = Utils::replace(result, "\\\"", "\"");
	result = Utils::replace(result, "\\\\", "\\");
	return result;
}

std::string Utils::file_name(const std::string& file_path)  
{  
   size_t last_slash = file_path.find_last_of("/\\");  
   if (last_slash == std::string::npos) return file_path;  
   return file_path.substr(last_slash + 1);  
}  

std::string Utils::file_name_no_ext(const std::string& file_name)  
{  
   size_t last_dot = file_name.find_last_of('.');  
   if (last_dot == std::string::npos) return file_name;  
   return file_name.substr(0, last_dot);  
}

std::string Utils::file_path_parent(const std::string& file_path)
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

std::string Utils::trim_quotes(const std::string& str)  
{  
   if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {  
       return str.substr(1, str.size() - 2);  
   }  
   return str;  
}  

std::string Utils::read_file(const std::string& filename)  
{  
   std::ifstream file(filename); // Ensure std::ifstream is included  
   if (!file.is_open()) {  
       throw std::runtime_error("Could not open file: " + filename);  
   }  
   std::stringstream buffer;  
   buffer << file.rdbuf();  
   return buffer.str();  
}

std::string Utils::replace_whitespace(const std::string& str, char replacement)
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

bool Utils::starts_with(const std::string& str, const std::string& prefix)
{
	if (str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0) {
		return true;
	}
	return false;
}

bool Utils::ends_with(const std::string& str, const std::string& suffix)
{
	if (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
		return true;
	}
	return false;
}

bool Utils::is_path_absolute(const std::string& path)
{
	if (path.size() >= 2 && std::isalpha(path[0]) && path[1] == ':') {
		return true; // Windows absolute path (e.g., C:\)
	}
	return false;
}
