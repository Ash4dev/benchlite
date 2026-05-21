cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# clangd uses compile_commands.json to understand include paths
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ensures all executables land in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(benchlite)

add_executable(main
  source/main.cpp
  source/utils/timer.cpp
  source/utils/event_manager.cpp
)

# CMAKE_CURRENT_SOURCE_DIR: builtin variable for full path of source directory
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
