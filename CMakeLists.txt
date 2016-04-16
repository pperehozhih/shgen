project(shgen)
cmake_minimum_required(VERSION 2.8)

option(BUILD_STANDALONE_SHGEN "Build standalone programs" OFF)

add_subdirectory(3rd/hlslparser)
add_subdirectory(3rd/hlsl2glsl)
add_subdirectory(3rd/glsl-optimizer)
include_directories(include
					3rd/hlslparser/src
					3rd/hlsl2glsl/include
					3rd/glsl-optimizer/src/glsl
					)
aux_source_directory(src SRC_SHGEN_LIST)

file(GLOB INC_SHGEN_LIST "include/*.h")

source_group("include" FILES ${INC_SHGEN_LIST})
source_group("source" FILES ${SRC_SHGEN_LIST})

set(COMPILER_FILES "src/main.cpp")

list(REMOVE_ITEM SRC_SHGEN_LIST ${COMPILER_FILES})

add_library(${PROJECT_NAME} ${SRC_SHGEN_LIST} ${INC_SHGEN_LIST})

target_link_libraries(${PROJECT_NAME} hlslparser hlsl2glsl glsl_optimizer)
add_dependencies(${PROJECT_NAME} hlslparser hlsl2glsl glsl_optimizer)

if (BUILD_STANDALONE_SHGEN)
   include_directories(3rd/picojson)
   add_executable(${PROJECT_NAME}-compiler ${COMPILER_FILES})
	target_link_libraries(${PROJECT_NAME}-compiler ${PROJECT_NAME})
	add_dependencies(${PROJECT_NAME}-compiler ${PROJECT_NAME})
endif(BUILD_STANDALONE_SHGEN)
