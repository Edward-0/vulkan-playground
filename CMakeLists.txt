cmake_minimum_required(VERSION 3.0.0)
project(vulkan-playground)
enable_language(C)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_C_FLAGS "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(OpenAL REQUIRED)
find_package(glfw3 REQUIRED CONFIG)

include_directories(${VULKAN_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIR})


function(add_spirv_shader SHADER_STAGE INPUT_FILE OUTPUT_FILE)
	add_custom_command(
		OUTPUT ${OUTPUT_FILE}
		COMMAND glslc -fshader-stage=${SHADER_STAGE} ${INPUT_FILE} -o ${OUTPUT_FILE}
		MAIN_DEPENDENCY ${INPUT_FILE}
	)
endfunction()

add_spirv_shader(vert "${CMAKE_SOURCE_DIR}/src/shader.vert" vert.spv)
add_spirv_shader(frag "${CMAKE_SOURCE_DIR}/src/shader.frag" frag.spv)

add_custom_target(shaders ALL DEPENDS vert.spv frag.spv)

add_executable(vulkan-playground src/main.c vert.spv frag.spv)

target_link_libraries(vulkan-playground glfw vulkan)

install(TARGETS vulkan-playground)
