cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(PROJECT_NAME q3d-editor)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_subdirectory(ext/q3d)

target_link_libraries(${PROJECT_NAME} q3d)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)