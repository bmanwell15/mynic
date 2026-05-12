cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(MynicAPI VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

add_library(mynic SHARED
    Mynic.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/AST.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/Interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/MynicLib.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/adapters/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/adapters/FileHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/adapters/objects.cpp
)

target_include_directories(mynic PRIVATE 
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/adapters
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/api
)

install(TARGETS mynic DESTINATION lib) # Libraries go in 'lib'
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ DESTINATION include FILES_MATCHING PATTERN "*.h")