cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(PROJECT_NAME q3d-editor)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

add_subdirectory(ext/q3d)