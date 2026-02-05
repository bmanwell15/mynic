cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(Mynic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

add_executable(mynic 
    main.cpp
    lexer.cpp
    ErrorHandler.cpp
    AST.cpp
    Mynic.cpp
    DecodedPacket.cpp
    Interpreter.cpp
)

# Enable sanitizers for GCC/Clang (REMOVE THESE 2 LINES FOR PRODUCTION!)
target_compile_options(mynic PRIVATE -fsanitize=address -g -O1 -fno-omit-frame-pointer)
target_link_options(mynic PRIVATE -fsanitize=address)