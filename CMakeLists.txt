cmake_minimum_required(VERSION 3.16)

project(IGR201)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_DIR}/include ${PROJECT_DIR}/dependencies/imgui ${PROJECT_DIR}/dependencies/glad)
set(PROJECT_SOURCE_DIR ${PROJECT_DIR}/src)

set(PROJECT_SOURCES
            ${PROJECT_SOURCE_DIR}/main.cpp
            ${PROJECT_SOURCE_DIR}/init.c
            ${PROJECT_SOURCE_DIR}/input.cpp
            ${PROJECT_SOURCE_DIR}/camera.cpp
            ${PROJECT_SOURCE_DIR}/planet.cpp
            dependencies/glad/glad.c)


add_subdirectory(${PROJECT_DIR}/dependencies/glfw)
 
include_directories(${PROJECT_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} glfw)