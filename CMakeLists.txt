cmake_minimum_required(VERSION 3.28)

project(CMScript)

include(path_config.cmake)
CMScriptSetup(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${CMSCRIPT_ROOT_SOURCE_PATH})
add_subdirectory(examples)
