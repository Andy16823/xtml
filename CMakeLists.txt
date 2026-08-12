cmake_minimum_required(VERSION 3.16)
project(xtml LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# So clangd (editor IntelliSense) can find the compile flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB_RECURSE XTML_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/xtml/*.cpp")
file(GLOB_RECURSE XTML_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/xtml/*.h")

add_executable(xtml ${XTML_SOURCES} ${XTML_HEADERS})

target_include_directories(xtml PRIVATE ${CMAKE_SOURCE_DIR}/xtml)

if(UNIX)
    # dlopen/dlsym for the plugin module loader
    target_link_libraries(xtml PRIVATE ${CMAKE_DL_LIBS})
endif()

# std::filesystem needs an extra library on GCC < 9
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(xtml PRIVATE stdc++fs)
endif()
