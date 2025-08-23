cmake_minimum_required(VERSION 3.10)

# Find Google Test
find_package(GTest CONFIG REQUIRED)
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found. Install via vcpkg: vcpkg install gtest")
    message(FATAL_ERROR "Google Test is required for unit tests.")
endif()

# Find SFML (already found in main CMakeLists.txt)
find_package(SFML COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Create test executable
add_executable(InputManagerTests
    main.cpp
    input/InputManagerTest.cpp
    # Add the actual source files we're testing
    ../src/input/InputManager.cpp
    ../src/core/Logger.cpp
)

# Include directories
target_include_directories(InputManagerTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries
target_link_libraries(InputManagerTests PRIVATE 
    GTest::gtest
    GTest::gtest_main
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
    nlohmann_json::nlohmann_json
)

# Set C++ standard
set_target_properties(InputManagerTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(InputManagerTests)
