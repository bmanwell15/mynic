cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(MynicCore VERSION 1.0)

# 1. Collect all your source files
file(GLOB_RECURSE SOURCES "*.cpp")

# On Windows, this creates .dll; on Linux, .so; on macOS, .dylib
add_library(mynic SHARED ${SOURCES})

target_include_directories(mynic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core)

# 4. Optional: Set specific C++ standard
target_compile_features(mynic PUBLIC cxx_std_20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

# add_executable(mynic 
#     .
# )

# Enable sanitizers for GCC/Clang (REMOVE THESE 2 LINES FOR PRODUCTION!)
# target_compile_options(mynic PRIVATE -fsanitize=address -g -O1 -fno-omit-frame-pointer)
# target_link_options(mynic PRIVATE -fsanitize=address)