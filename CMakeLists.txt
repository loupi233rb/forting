cmake_minimum_required(VERSION 3.16)

project(forting VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)

add_executable(forting src/main.cpp
    src/convert.cpp
    src/fortingcore.cpp
    src/SyntaxParser.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

# 定义DEBUG宏
target_compile_definitions(forting PRIVATE $<$<CONFIG:Debug>:DEBUG>)


