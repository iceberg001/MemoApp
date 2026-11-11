cmake_minimum_required(VERSION 3.15)

project(MemoApp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_GENERATOR_TOOLSET)
    set(CMAKE_GENERATOR_TOOLSET "v143" CACHE STRING "VS Toolset")
endif()

add_definitions(-DUNICODE -D_UNICODE)
add_compile_options(/W3 /WX-)

# DuiLib 静态编译（编进 exe，输出单文件程序）
set(DUILIB_STATIC ON)

# 第三方库
add_subdirectory(third_party/DuiLib)

# 应用
add_subdirectory(src)
