#pragma once  
#include <string>  
#include <map>  
#include <vector>  

using namespace std;  

enum DataType
{
	DT_STRING,
	DT_NUMBER,
	DT_BOOL,
	DT_UNKNOWN
};

struct var {
	string value;
	DataType type;
};

class Vars  
{  
public: 
static string trim_var(const string& var);  
static tuple<string, string> parse_var(const std::string& line);  
static map<string, var> parse_vars(const std::string& content);  
static vector<string> parse_tokens(const string& expr, const char* ops, bool addop);  
static string replace_vars(string& content, const map<string, var>& vars);
static string preprocess_content(const string& content);
static bool is_string_expr(const string& expr, const map<string, var>& vars);
static bool is_string_expr(vector<string>& tokens, const map<string, var>& vars);
static bool is_numeric_expr(vector<string>& tokens, const map<string, var>& vars);
static var eval_expr(const string& expr, const map<string, var>& vars);
static var eval_str_expr(vector<string>& tokens, const map<string, var>& vars);
static var eval_num_expr(vector<string>& tokens, const map<string, var>& vars);
};
