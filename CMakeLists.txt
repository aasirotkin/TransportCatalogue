# Check cmake version
cmake_minimum_required(VERSION 3.0.0)

# Sets the name of the project, and stores it in the variable PROJECT_NAME
project(Transport VERSION 0.0.1 LANGUAGES CXX)

# The C++ standard whose features are requested to build this target
set(CMAKE_CXX_STANDARD 17)

# Find an external project, and load its settings
# This command will find Protobuf and Threads
# For using this set CMAKE_PREFIX_PATH
# cmake . -DCMAKE_PREFIX_PATH=/path/to/ProtoBuf/package/
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# Execute protoc.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport.proto)

# If this property is set to ON, then the value of the CXX_STANDARD target property is treated as a requirement
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This property specifies whether compiler specific extensions should be used, On by default
set(CMAKE_CXX_EXTENSIONS OFF)

# Collects the names of all the source files in the specified directory and stores the list in the variable provided
aux_source_directory(src/ SRC_LIST)

# Add an executable to the project using the specified source files
add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${SRC_LIST})

target_include_directories(${PROJECT_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(${PROJECT_NAME} "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

# CXXFLAGS - This is a CMake Environment Variable
set (CMAKE_CXX_FLAGS "-Wall")

if (NOT CMAKE_SYSTEM_NAME MATCHES ".*WIN.*")
    # Specify libraries or flags to use when linking a given target and/or its dependents
    # tbb - is an intel Threading Building Blocks
    target_link_libraries(${PROJECT_NAME} -ltbb -lpthread)

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wpedantic")
endif()
message(STATUS "${CMAKE_SYSTEM_NAME} operation system")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

# Adds -D define flags to the compilation of source files
add_definitions(-D_DEBUG -D_SIROTKIN_HOME_TESTS_)
