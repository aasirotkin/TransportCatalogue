# Check cmake version
cmake_minimum_required(VERSION 3.0.0)

# Sets the name of the project, and stores it in the variable PROJECT_NAME
project(Transport VERSION 0.0.1 LANGUAGES CXX)

# The C++ standard whose features are requested to build this target
set(CMAKE_CXX_STANDARD 17)

# If this property is set to ON, then the value of the CXX_STANDARD target property is treated as a requirement
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This property specifies whether compiler specific extensions should be used, On by default
set(CMAKE_CXX_EXTENSIONS OFF)

# Collects the names of all the source files in the specified directory and stores the list in the variable provided
aux_source_directory(src/ SRC_LIST)

# Add an executable to the project using the specified source files
add_executable(${PROJECT_NAME} ${SRC_LIST})

# Specify libraries or flags to use when linking a given target and/or its dependents
# tbb - is an intel Threading Building Blocks
target_link_libraries(${PROJECT_NAME} -ltbb -lpthread)

# CXXFLAGS - This is a CMake Environment Variable
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
