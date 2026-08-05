cmake_minimum_required(VERSION 3.14)
project(quantflow-sdk VERSION 0.1.0 LANGUAGES CXX)

add_library(quantflow INTERFACE)
target_include_directories(quantflow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(quantflow INTERFACE cxx_std_17)

# Require nlohmann/json
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(quantflow INTERFACE nlohmann_json::nlohmann_json)
endif()
