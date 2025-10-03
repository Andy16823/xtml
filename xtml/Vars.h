#pragma once  
#include <string>  
#include <map>  
#include <vector>  

enum DataType
{
	DT_STRING,
	DT_NUMBER,
	DT_BOOL,
	DT_ARRAY,
	DT_UNKNOWN
};

struct var {
	std::string value;
	DataType type;
	std::vector<var> array; // For DT_ARRAY type
};

class Vars  
{  
public: 
	static std::string trim_var(const std::string& var);  
	static std::tuple<std::string, std::string> parse_var(const std::string& line);
	static std::vector<std::string> parse_tokens(const std::string& expr, const char* ops, bool addop);  
	static std::string replace_vars(std::string& content, const std::map<std::string, var>& vars);
	static std::string preprocess_content(const std::string& content);
	static bool is_string_expr(const std::string& expr, const std::map<std::string, var>& vars);
	static bool is_string_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static bool is_numeric_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static bool is_bool_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static bool is_function_expr(std::vector<std::string>& tokens);
	static std::vector<std::string> parse_top_level_tokens(const std::string& expr);
	static var eval_expr(const std::string& expr, const std::map<std::string, var>& vars);
	static var eval_str_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static var eval_num_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static var eval_func_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars);
	static std::map<std::string, var> merge_vars(const std::map<std::string, var>& arr1, const std::map<std::string, var>& arr2);

	static bool is_function_expr(const std::string& token);
	static var eval_func_expr(const std::string& token, const std::map<std::string, var>& vars);
	static bool is_array_expr(const std::string& token);
	static var eval_array_expr(const std::string& token, const std::map<std::string, var>& vars);


};
