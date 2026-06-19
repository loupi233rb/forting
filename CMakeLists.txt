cmake_minimum_required(VERSION 3.16)

project(forting VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)

file(GLOB CPP_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(forting ${CPP_SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/include)

# 定义DEBUG宏
target_compile_definitions(forting PRIVATE $<$<CONFIG:Debug>:DEBUG>)