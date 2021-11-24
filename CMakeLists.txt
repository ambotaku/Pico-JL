cmake_minimum_required(VERSION 3.13)


include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-jl 
    LANGUAGES C CXX ASM 
    VERSION 0.1) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_definitions(-DRP2040)

add_executable(${CMAKE_PROJECT_NAME}
    src/jl-context.cpp
    src/jl-func.cpp
    src/jl-scope.cpp
    src/jl-value.cpp
    src/jl.cpp
    src/jli.cpp
 )


target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib)

pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
