#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <map>
#include <tuple>
#include "Utils.h"
#include "Vars.h"

#define VERSION "0.1.0"

using namespace std;


void create_file(string& content, const string& output_path) {
	std::ofstream file(output_path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not create file: " + output_path);
	}
	file << content;
	file.close();
}

void action_build(const string& file_path) {
	Utils::print_ln(string("Building file ") + file_path);
	auto content = Utils::read_file(file_path);
	auto vars = Vars::parse_vars(content);
	Utils::print_ln("Parsed Variables:");
	for (const auto& var : vars) {
		const string& key = var.first;
		const string& value = var.second.value;
		Utils::print_ln("Key: " + key + ", Value: " + value);
	}

	content = Vars::replace_vars(content, vars);
	Utils::print_ln("Final Content:");
	Utils::print_ln(content);
}

int main(int argc, char* argv[])  
{  
   if (argc < 2) {  
	   Utils::printerr_ln("Usage: <command> <file_path>");
       return 1;  
   }  

   string command = argv[1];

   if (command == "version") {
	   Utils::print_ln(string("xtml version: ") + VERSION);
	   return 0;
   }
   else if (command == "build") {
	   action_build(argv[2]);
   }


   return 0;  
}