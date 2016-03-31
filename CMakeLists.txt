project(shgen)
cmake_minimum_required(VERSION 2.8)

option(BUILD_STANDALONE_SHGEN "Build standalone programs" OFF)

add_subdirectory(3rd/hlslparser)
add_subdirectory(3rd/hlsl2glsl)
add_subdirectory(3rd/glsl-optimizer)
include_directories(include
					3rd/hlslparser/src
					3rd/hlsl2glsl/include
					)
aux_source_directory(src SRC_LIST)

list(REMOVE_ITEM SRC_LIST "src/main.cpp")

add_library(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} hlslparser hlsl2glsl)
add_dependencies(${PROJECT_NAME} hlslparser hlsl2glsl)

if (BUILD_STANDALONE_SHGEN)
	add_executable(${PROJECT_NAME}-compiler "src/main.cpp")
	target_link_libraries(${PROJECT_NAME}-compiler ${PROJECT_NAME})
	add_dependencies(${PROJECT_NAME}-compiler ${PROJECT_NAME})
endif(BUILD_STANDALONE_SHGEN)

