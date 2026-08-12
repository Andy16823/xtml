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
#include "Globals.h"
#include "Utils.h"
#include "Vars.h"
#include "Core.h"
#include "FunctionRegistry.h"
#include "ModuleStd.h"
#include "Module.h"
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#endif

#define VERSION "0.0.1.0"

namespace fs = std::filesystem;

FunctionRegistry g_functionRegistry;

typedef Module* (*CreateModuleFunc)();

#if defined(_WIN32)
static constexpr const char* MODULE_EXTENSION = ".dll";
#elif defined(__APPLE__)
static constexpr const char* MODULE_EXTENSION = ".dylib";
#else
static constexpr const char* MODULE_EXTENSION = ".so";
#endif

void loadModulesFromFolder(const std::string& folderPath) {
	for (const auto& entry : fs::directory_iterator(folderPath)) {
		if (entry.path().extension() == MODULE_EXTENSION) {
			std::string dllPath = entry.path().string();

#if defined(_WIN32)
			HMODULE hModule = LoadLibraryA(dllPath.c_str());
			if (!hModule) {
				std::cerr << "Fehler: konnte " << dllPath << " nicht laden." << std::endl;
				continue;
			}

			auto createModule = (CreateModuleFunc)GetProcAddress(hModule, "CreateModule");
			if (!createModule) {
				std::cerr << "Fehler: CreateModule nicht gefunden in " << dllPath << std::endl;
				FreeLibrary(hModule);
				continue;
			}
#else
			void* hModule = dlopen(dllPath.c_str(), RTLD_NOW);
			if (!hModule) {
				std::cerr << "Fehler: konnte " << dllPath << " nicht laden: " << dlerror() << std::endl;
				continue;
			}

			auto createModule = (CreateModuleFunc)dlsym(hModule, "CreateModule");
			if (!createModule) {
				std::cerr << "Fehler: CreateModule nicht gefunden in " << dllPath << std::endl;
				dlclose(hModule);
				continue;
			}
#endif

			Module* plugin = createModule();
			plugin->RegisterFunctions(g_functionRegistry);
		}
	}
}

std::string getExeDir() {
#if defined(_WIN32)
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	fs::path exePath(buffer);
#else
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len == -1) {
		return fs::current_path().string();
	}
	buffer[len] = '\0';
	fs::path exePath(buffer);
#endif
	return exePath.parent_path().string();
}

void action_build(const std::string& file_path) {

	std::string path = file_path;
	if (Utils::isAbsolute(path) == false) {
		path = (fs::current_path() / file_path).string();
	}

	// Get the raw file name
	auto file_name = Utils::fileName(path);
	file_name = Utils::fileNameNoExt(file_name) + ".html";

	// Get the file directory
	auto file_dir = Utils::filePathParent(path);
	auto output_path = (fs::path(file_dir) / file_name).string();

	// Build the file and write to output
	std::map<std::string, var> vars;
	//auto content = Core::build_file(path, vars);
	auto content = Core::buildFile(path); // New AST-based builder (not finished yet)
	Core::writeFile(content, output_path);
}

int main(int argc, char* argv[])  
{  
	auto exe_path = getExeDir();
	auto modules_path = (fs::path(exe_path) / "modules").string();
	if (!fs::exists(modules_path)) {
		fs::create_directory(modules_path);
	}
	loadModulesFromFolder(modules_path);

	// Register standard functions
	ModuleStd stdModule;
	stdModule.RegisterFunctions(g_functionRegistry);
	

	if (argc < 2) {  
		Utils::printerrLn("Usage: <command> <file_path>");
		return 1;  
	}  

	std::string command = argv[1];

	if (command == "version") {
		Utils::printLn(std::string("xtml version: ") + VERSION);
		return 0;
	}
	else if (command == "build") {
		action_build(argv[2]);
	}


	return 0;  
}