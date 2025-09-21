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
static std::string file_name(const std::string& file_path);  
static std::string file_name_no_ext(const std::string& file_name);  
static std::string trim(const std::string& str);  
static std::string trim_quotes(const std::string& str);  
static std::string read_file(const std::string& filename);  
static std::string replace_whitespace(const std::string& str, char replacement);  
static std::vector<std::string> split(const std::string& str, char delimiter);  
};
