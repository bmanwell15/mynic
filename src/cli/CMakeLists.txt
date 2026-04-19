cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(MynicCLI VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Fixed typo: it's usually -O0 (the letter O, not zero) for no optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

add_executable(mynic_cli 
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/Mynic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/AST.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/Interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/lexer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../adapters/json.cpp
)

# 1. Point to the library file
# target_link_libraries(mynic_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmynic.so)
# target_link_libraries(mynic_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmynic_adapters.so)

# 2. FIX: Tell the compiler WHERE the .h files are actually located
# We need to include both the current dir (.) and the core dir (../core)
target_include_directories(mynic_cli PRIVATE 
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/adapters
)

# 3. Installation logic (Optional but kept for your structure)
# install(TARGETS mynic_cli DESTINATION bin) # CLI apps usually go in 'bin', not 'lib'
# install(DIRECTORY ../core/ DESTINATION include FILES_MATCHING PATTERN "*.h")
# install(DIRECTORY ../adapters/ DESTINATION include FILES_MATCHING PATTERN "*.h")