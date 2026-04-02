# ReflectScan.cmake
# CMake module providing reflect_scan_directory() — a single function that
# wires the batch reflection scanner into any CMake project.
#
# Place this file alongside reflect_scan.py and wamr_codegen.py so the
# module can locate the Python tools by relative path automatically.
#
# Usage
# -----
#   list(APPEND CMAKE_MODULE_PATH "<path/to/tools>")
#   include(ReflectScan)
#
#   reflect_scan_directory(
#       TARGET          MyHostBindings          # required — name of generated STATIC lib
#       SCAN_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/host
#       OUT_DIR         ${CMAKE_CURRENT_BINARY_DIR}/reflect_gen
#       # --- optional ---
#       STD             c++20
#       BOOTSTRAP_FUNC  register_all_script_bindings
#       BOOTSTRAP_OUT   ${CMAKE_CURRENT_BINARY_DIR}/reflect_gen/ReflectionBootstrap.gen.cpp
#       EXT             h               # comma-separated, e.g. "h,cpp"
#       INCLUDE_DIRS    ${CMAKE_CURRENT_SOURCE_DIR}/host
#                       ${PROJECT_SOURCE_DIR}/inc
#       EXCLUDE         "vendor/**"     # relative to SCAN_DIR, fnmatch syntax
#       CLANGXX         clang++
#   )
#
# After the call the following are available in the caller's scope:
#   ${TARGET}_BOOTSTRAP_H   — path to the generated bootstrap header
#   ${TARGET}_BOOTSTRAP_CPP — path to the generated bootstrap .cpp
#
# The generated STATIC library target (${TARGET}) contains:
#   - one .reflect.cpp per annotated header/source file found
#   - the aggregated ReflectionBootstrap.gen.cpp
#
# It exposes all include directories listed in INCLUDE_DIRS plus OUT_DIR as
# PUBLIC include directories, so consumers only need:
#   target_link_libraries(MyExe PRIVATE MyHostBindings)
#
# Two-phase operation
# -------------------
# Phase 1 — CMake configure time:
#   execute_process() runs reflect_scan.py --dry-run --emit-cmake, which uses
#   a fast text grep (no clang) to discover annotated files and writes a
#   reflect_sources.cmake fragment listing the expected output paths.
#   CMake includes this fragment to learn the OUTPUT list for add_custom_command.
#
# Phase 2 — CMake build time:
#   add_custom_command() re-runs reflect_scan.py (without --dry-run) on every
#   build, invoking wamr_codegen.py (clang AST) per file and regenerating the
#   bootstrap.  CMake's dependency tracking re-triggers this only when any
#   annotated input file or the tool itself changes.

cmake_minimum_required(VERSION 3.20)

# Guard against multiple inclusion
if(DEFINED _REFLECT_SCAN_CMAKE_INCLUDED)
    return()
endif()
set(_REFLECT_SCAN_CMAKE_INCLUDED TRUE)

function(reflect_scan_directory)
    # -----------------------------------------------------------------------
    # Parse arguments
    # -----------------------------------------------------------------------
    cmake_parse_arguments(RSC
        ""
        "TARGET;SCAN_DIR;OUT_DIR;STD;BOOTSTRAP_FUNC;BOOTSTRAP_OUT;EXT;CLANGXX"
        "INCLUDE_DIRS;EXCLUDE"
        ${ARGN}
    )

    if(NOT DEFINED RSC_TARGET)
        message(FATAL_ERROR "reflect_scan_directory: TARGET is required")
    endif()
    if(NOT DEFINED RSC_SCAN_DIR)
        message(FATAL_ERROR "reflect_scan_directory: SCAN_DIR is required")
    endif()

    # Defaults
    if(NOT DEFINED RSC_OUT_DIR)
        set(RSC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/reflect_gen")
    endif()
    if(NOT DEFINED RSC_STD)
        set(RSC_STD "c++20")
    endif()
    if(NOT DEFINED RSC_BOOTSTRAP_FUNC)
        set(RSC_BOOTSTRAP_FUNC "register_all_script_bindings")
    endif()
    if(NOT DEFINED RSC_BOOTSTRAP_OUT)
        set(RSC_BOOTSTRAP_OUT "${RSC_OUT_DIR}/ReflectionBootstrap.gen.cpp")
    endif()
    if(NOT DEFINED RSC_EXT)
        set(RSC_EXT "h")
    endif()
    if(NOT DEFINED RSC_CLANGXX)
        set(RSC_CLANGXX "clang++")
    endif()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    # Locate tool scripts — live in the same directory as this .cmake file
    get_filename_component(_rsc_tools_dir "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY)
    set(_rsc_scan_tool "${_rsc_tools_dir}/reflect_scan.py")

    if(NOT EXISTS "${_rsc_scan_tool}")
        message(FATAL_ERROR
            "reflect_scan_directory: reflect_scan.py not found at ${_rsc_scan_tool}\n"
            "Ensure ReflectScan.cmake and reflect_scan.py reside in the same directory.")
    endif()

    set(_rsc_cmake_fragment "${RSC_OUT_DIR}/reflect_sources.cmake")

    # -----------------------------------------------------------------------
    # Build the common part of the command (shared between dry-run and full)
    # -----------------------------------------------------------------------
    set(_rsc_cmd
        "${Python3_EXECUTABLE}" "${_rsc_scan_tool}"
        --scan-dir        "${RSC_SCAN_DIR}"
        --out-dir         "${RSC_OUT_DIR}"
        --std             "${RSC_STD}"
        --bootstrap-func  "${RSC_BOOTSTRAP_FUNC}"
        --bootstrap-out   "${RSC_BOOTSTRAP_OUT}"
        --ext             "${RSC_EXT}"
        --clangxx         "${RSC_CLANGXX}"
        --emit-cmake      "${_rsc_cmake_fragment}"
    )
    foreach(_inc ${RSC_INCLUDE_DIRS})
        list(APPEND _rsc_cmd -I "${_inc}")
    endforeach()
    foreach(_excl ${RSC_EXCLUDE})
        list(APPEND _rsc_cmd --exclude "${_excl}")
    endforeach()

    # -----------------------------------------------------------------------
    # Phase 1 — configure-time dry run (text grep only, fast)
    # -----------------------------------------------------------------------
    file(MAKE_DIRECTORY "${RSC_OUT_DIR}")

    execute_process(
        COMMAND           ${_rsc_cmd} --dry-run
        WORKING_DIRECTORY "${RSC_SCAN_DIR}"
        RESULT_VARIABLE   _rsc_dry_result
        OUTPUT_VARIABLE   _rsc_dry_output
        ERROR_VARIABLE    _rsc_dry_error
    )

    if(NOT _rsc_dry_result EQUAL 0)
        message(WARNING
            "reflect_scan_directory: configure-time dry-run returned ${_rsc_dry_result}\n"
            "${_rsc_dry_error}")
    else()
        message(STATUS "${_rsc_dry_output}")
    endif()

    # Load the generated cmake fragment to get REFLECT_* variables
    if(EXISTS "${_rsc_cmake_fragment}")
        include("${_rsc_cmake_fragment}")
    else()
        # No annotated files found — provide empty placeholders
        set(REFLECT_SCAN_INPUT_FILES "")
        set(REFLECT_GEN_HEADERS      "")
        set(REFLECT_GEN_SOURCES      "")
        set(REFLECT_BOOTSTRAP_CPP    "${RSC_BOOTSTRAP_OUT}")
        set(REFLECT_BOOTSTRAP_H      "${RSC_BOOTSTRAP_OUT}")
    endif()

    # -----------------------------------------------------------------------
    # Phase 2 — build-time full codegen via add_custom_command
    # -----------------------------------------------------------------------
    add_custom_command(
        OUTPUT
            ${REFLECT_GEN_HEADERS}
            ${REFLECT_GEN_SOURCES}
            "${REFLECT_BOOTSTRAP_CPP}"
            "${REFLECT_BOOTSTRAP_H}"
        COMMAND ${_rsc_cmd}
        DEPENDS
            ${REFLECT_SCAN_INPUT_FILES}
            "${_rsc_scan_tool}"
        COMMENT
            "[ReflectScan] Generating reflection code for ${RSC_SCAN_DIR}"
        VERBATIM
    )

    add_custom_target(${RSC_TARGET}_codegen
        DEPENDS
            ${REFLECT_GEN_SOURCES}
            ${REFLECT_GEN_HEADERS}
            "${REFLECT_BOOTSTRAP_CPP}"
    )

    # -----------------------------------------------------------------------
    # Static library — generated .reflect.cpp + bootstrap
    # -----------------------------------------------------------------------
    add_library(${RSC_TARGET} STATIC
        ${REFLECT_GEN_SOURCES}
        "${REFLECT_BOOTSTRAP_CPP}"
    )
    add_dependencies(${RSC_TARGET} ${RSC_TARGET}_codegen)

    target_include_directories(${RSC_TARGET}
        PUBLIC
            "${RSC_OUT_DIR}"
            ${RSC_INCLUDE_DIRS}
    )

    # -----------------------------------------------------------------------
    # Expose bootstrap paths to the calling scope
    # -----------------------------------------------------------------------
    set(${RSC_TARGET}_BOOTSTRAP_H   "${REFLECT_BOOTSTRAP_H}"   PARENT_SCOPE)
    set(${RSC_TARGET}_BOOTSTRAP_CPP "${REFLECT_BOOTSTRAP_CPP}" PARENT_SCOPE)

    message(STATUS
        "[ReflectScan] target '${RSC_TARGET}' configured, "
        "${REFLECT_GEN_SOURCES}" " source(s) discovered")
endfunction()
