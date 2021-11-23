cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(jl 
    LANGUAGES C CXX ASM 
    VERSION 0.1) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(jl
    jl-context.cpp
    jl-func.cpp
    jl-scope.cpp
    jl-value.cpp
    jl.cpp
    jli.cpp
 )


target_link_libraries(jl pico_stdlib)

pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
