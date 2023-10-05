cmake_minimum_required(VERSION 3.18)

project(bushing-auto C CXX)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

list (APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    $ENV{HOME}/.local/share/cmake-3.20/Modules
    )

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "------------- RELEASE --------------------")
    add_compile_options("-Wno-error=stringop-overflow=")
    message(STATUS "------------- RELEASE --------------------")
endif(CMAKE_BUILD_TYPE STREQUAL "Release")


add_subdirectory(src)

enable_testing()
