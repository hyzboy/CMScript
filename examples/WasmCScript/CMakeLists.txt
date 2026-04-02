cmake_minimum_required(VERSION 3.28)

project(WasmCScriptExample LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# ---------------------------------------------------------------------------
# Reflection batch codegen via ReflectScan module
# ---------------------------------------------------------------------------
set(_tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../src/WasmVM/tools)
list(APPEND CMAKE_MODULE_PATH "${_tools_dir}")
include(ReflectScan)

reflect_scan_directory(
    TARGET       WasmCScriptHost
    SCAN_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/host
    OUT_DIR      ${CMAKE_CURRENT_BINARY_DIR}/reflect_gen
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/host
        ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
)

# Add the hand-written host sources to the same library
target_sources(WasmCScriptHost PRIVATE
    host/GameHost.cpp
)

# Link required libraries from parent project
target_link_libraries(WasmCScriptHost PRIVATE
    CMScript
    CMCore
    CMPlatform
    CMUtil
)

# Optional: switch on for static-initializer auto registration.
# target_compile_definitions(WasmCScriptHost PUBLIC H_REFLECT_AUTO_REGISTER=1)
