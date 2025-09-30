#pragma once  
#include <string>  
#include <vector>  

class Utils  
{  
public:  
static bool is_number(const std::string& s);  
static bool is_alpha(const std::string& s);  
static bool is_string(const std::string& s);  
static void print_ln(const std::string& str);  
static void printerr_ln(const std::string& str);  
static void throw_err(const std::string& str, const std::string& stack_trace = "");
static std::string escape_str(const std::string& str);
static std::string file_name(const std::string& file_path);  
static std::string file_name_no_ext(const std::string& file_name);  
static std::string file_path_parent(const std::string& file_path);
static std::string trim(const std::string& str);  
static std::string trim_quotes(const std::string& str);  
static std::string read_file(const std::string& filename);  
static std::string replace_whitespace(const std::string& str, char replacement);  
static std::vector<std::string> split(const std::string& str, char delimiter);  
static std::string replace(const std::string& str, const std::string& from, const std::string& to);
static bool starts_with(const std::string& str, const std::string& prefix);
static bool ends_with(const std::string& str, const std::string& suffix);
static bool is_path_absolute(const std::string& path);


// Parsers
static std::string parse_parantheses(const std::string& str); // Parse content within the first matching parentheses
};
