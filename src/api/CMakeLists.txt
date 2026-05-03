cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(MynicAPI VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

add_library(mynic_api SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/Mynic.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../core/AST.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/Interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/MynicLib.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../adapters/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../adapters/FileHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../adapters/objects.cpp
)

target_include_directories(mynic_api PRIVATE 
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/adapters
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/api
)

install(TARGETS mynic_api DESTINATION lib) # Libraries go in 'lib'
install(DIRECTORY ../core/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../adapters/ DESTINATION include FILES_MATCHING PATTERN "*.h")