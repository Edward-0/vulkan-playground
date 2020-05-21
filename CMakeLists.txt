cmake_minimum_required(VERSION 3.0.0)
project(vulkan-playground)
enable_language(C)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_C_FLAGS "-g" )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED CONFIG)

include_directories(${VULKAN_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIR})

add_executable(vulkan-playground src/main.c)

target_link_libraries(vulkan-playground glfw vulkan)

install(TARGETS vulkan-playground)
