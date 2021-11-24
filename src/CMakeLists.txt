cmake_minimum_required(VERSION 3.13)

project(jl 
    LANGUAGES C CXX ASM 
    VERSION 0.1) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(${CMAKE_PROJECT_NAME}
    jl-context.cpp
    jl-func.cpp
    jl-scope.cpp
    jl-value.cpp
    jl.cpp
    jli.cpp
 )
