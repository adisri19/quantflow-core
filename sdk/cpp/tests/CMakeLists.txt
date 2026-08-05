cmake_minimum_required(VERSION 3.14)
project(quantflow_cpp_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# Fetch doctest
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(json doctest)

# Include parent directory for quantflow.h
include_directories(${CMAKE_SOURCE_DIR}/..)

# Automatically find all test source files
# main.cpp provides the doctest entry point
# test_*.cpp files contain the actual tests
file(GLOB TEST_SOURCES
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/test_*.cpp"
)

# Log discovered test files for debugging
message(STATUS "Discovered test files:")
foreach(TEST_FILE ${TEST_SOURCES})
    message(STATUS "  - ${TEST_FILE}")
endforeach()

add_executable(quantflow_test ${TEST_SOURCES})
target_link_libraries(quantflow_test PRIVATE nlohmann_json::nlohmann_json doctest::doctest)

# Enable CTest integration
enable_testing()
add_test(NAME quantflow_sdk_tests COMMAND quantflow_test)
