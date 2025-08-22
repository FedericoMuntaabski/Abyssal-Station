cmake_minimum_required(VERSION 3.10)
project(AbyssalStation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If user passed a toolchain (for example vcpkg), CMake will use it. This
# project expects SFML to be available via find_package. Recommend using vcpkg
# if SFML isn't installed system-wide.

message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

## Prefer the imported config targets provided by modern SFML packages (vcpkg installs SFML 3.x)
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)
if(NOT SFML_FOUND)
    message(STATUS "SFML not found by CMake. If you installed SFML via vcpkg, run CMake with -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake")
    message(FATAL_ERROR "SFML (graphics, window, system) is required. Set SFML_DIR or use vcpkg.")
endif()

add_executable(AbyssalStation
    src/main.cpp
    src/core/Game.cpp
    src/core/Game.h
)

target_include_directories(AbyssalStation PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AbyssalStation PRIVATE SFML::Graphics SFML::Window SFML::System)
