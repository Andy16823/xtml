#pragma once  
#include <string>  
#include <vector>  
#include "Vars.h"

/// <summary>
/// Utility Functions
/// </summary>
class Utils  
{  
public:  

	/// <summary>
	/// Check if string represents a number
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static bool isNumber(const std::string& s);  

	/// <summary>
	/// Check if string contains only alphabetic characters
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static bool isAlpha(const std::string& s);  

	/// <summary>
	/// Check if string is a quoted string
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static bool isString(const std::string& s);  

	/// <summary>
	/// Check if string represents a boolean value
	/// </summary>
	/// <param name="s"></param>
	/// <returns></returns>
	static bool isBool(const std::string& s);

	/// <summary>
	/// Print string with newline
	/// </summary>
	/// <param name="str"></param>
	static void printLn(const std::string& str);  

	/// <summary>
	/// Print error string with newline
	/// </summary>
	/// <param name="str"></param>
	static void printerrLn(const std::string& str);  

	/// <summary>
	/// Throw runtime error with message and optional stack trace
	/// </summary>
	/// <param name="str"></param>
	/// <param name="stack_trace"></param>
	static void throwErr(const std::string& str, const std::string& stack_trace = "");

	/// <summary>
	/// Escape special characters in a string
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	static std::string escapeStr(const std::string& str);

	/// <summary>
	/// Retrieve the file name from a file path
	/// </summary>
	/// <param name="file_path"></param>
	/// <returns></returns>
	static std::string fileName(const std::string& file_path);  

	/// <summary>
	/// Retrieve the file name without extension from a file name
	/// </summary>
	/// <param name="file_name"></param>
	/// <returns></returns>
	static std::string fileNameNoExt(const std::string& file_name);

	/// <summary>
	/// Retrieve the parent directory from a file path
	/// </summary>
	/// <param name="file_path"></param>
	/// <returns></returns>
	static std::string filePathParent(const std::string& file_path);

	/// <summary>
	/// Trim whitespace from both ends of a string
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	static std::string trim(const std::string& str);  

	/// <summary>
	/// Trim surrounding quotes from a string
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	static std::string trimQuotes(const std::string& str);  

	/// <summary>
	/// Read entire file content into a string
	/// </summary>
	/// <param name="filename"></param>
	/// <returns></returns>
	static std::string readFile(const std::string& filename);  

	/// <summary>
	/// Replace all whitespace characters in a string with a specified character
	/// </summary>
	/// <param name="str"></param>
	/// <param name="replacement"></param>
	/// <returns></returns>
	static std::string replaceWhitespace(const std::string& str, char replacement); 

	/// <summary>
	/// Split a string by a specified delimiter character
	/// </summary>
	/// <param name="str"></param>
	/// <param name="delimiter"></param>
	/// <returns></returns>
	static std::vector<std::string> split(const std::string& str, char delimiter);  

	/// <summary>
	/// Replace all occurrences of a substring with another substring
	/// </summary>
	/// <param name="str"></param>
	/// <param name="from"></param>
	/// <param name="to"></param>
	/// <returns></returns>
	static std::string replace(const std::string& str, const std::string& from, const std::string& to);

	/// <summary>
	/// Check if a string starts with a specified prefix
	/// </summary>
	/// <param name="str"></param>
	/// <param name="prefix"></param>
	/// <returns></returns>
	static bool startsWith(const std::string& str, const std::string& prefix);

	/// <summary>
	/// Check if a string ends with a specified suffix
	/// </summary>
	/// <param name="str"></param>
	/// <param name="suffix"></param>
	/// <returns></returns>
	static bool endsWith(const std::string& str, const std::string& suffix);

	/// <summary>
	/// Check if a file path is absolute
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static bool isAbsolute(const std::string& path);

	/// <summary>
	/// Generate a UUID string
	/// </summary>
	/// <returns></returns>
	static std::string generateUuid();

	/// <summary>
	/// Predict the data type of a variable from its string representation
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	static DataType predictVarType(const std::string& value);

	/// <summary>
	/// Convert a string to a boolean value
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	static bool toBool(const std::string& value);

	/// <summary>
	/// Parse content within the first matching parentheses
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	static std::string parseParantheses(const std::string& str); // Parse content within the first matching parentheses
};
