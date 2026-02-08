cmake_minimum_required(VERSION 3.28)

option(CMSCRIPT_ENABLE_WASM "Build WASM VM backend" ON)
option(CMSCRIPT_ENABLE_WAMR "Build WAMR backend" ON)
option(CMSCRIPT_ENABLE_WASMEDGE "Build WasmEdge backend" ON)

if(NOT CMSCRIPT_ENABLE_WASM)
    return()
endif()

# WASM VM Public Headers
set(WASM_VM_PUBLIC_HEADERS
    ${CMSCRIPT_ROOT_INCLUDE_PATH}/hgl/wasm/VM.h
    ${CMSCRIPT_ROOT_INCLUDE_PATH}/hgl/wasm/WasmVM.h
    ${CMSCRIPT_ROOT_INCLUDE_PATH}/hgl/wasm/WasmModule.h
    ${CMSCRIPT_ROOT_INCLUDE_PATH}/hgl/wasm/WasmContext.h
)

# WASM VM Core Files
set(WASM_VM_CORE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/WasmVM.cpp
)

set(WASM_VM_SOURCES ${WASM_VM_PUBLIC_HEADERS} ${WASM_VM_CORE_FILES})
set(WASM_VM_INCLUDE_DIRS ${CMSCRIPT_ROOT_INCLUDE_PATH})
set(WASM_VM_LIBRARIES "")

# WAMR Backend
if(CMSCRIPT_ENABLE_WAMR)
    message(STATUS "Building with WAMR backend")
    
    # Configure WAMR
    set(WAMR_BUILD_INTERP 1)
    set(WAMR_BUILD_FAST_INTERP 1)
    set(WAMR_BUILD_AOT 0)
    set(WAMR_BUILD_JIT 0)
    set(WAMR_BUILD_LIBC_BUILTIN 1)
    set(WAMR_BUILD_LIBC_WASI 0)
    set(WAMR_BUILD_MULTI_MODULE 0)
    set(WAMR_BUILD_LIB_PTHREAD 0)
    set(WAMR_BUILD_MINI_LOADER 0)
    set(WAMR_BUILD_SIMD 0)
    set(WAMR_BUILD_REF_TYPES 0)
    
    # Add WAMR as subdirectory
    set(WAMR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/wasm-micro-runtime)
    
    # Include WAMR build scripts
    include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
    
    set(WAMR_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/WAMR/WAMRModule.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WAMR/WAMRModule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WAMR/WAMRContext.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WAMR/WAMRContext.cpp
    )
    
    list(APPEND WASM_VM_SOURCES ${WAMR_SOURCES})
    list(APPEND WASM_VM_INCLUDE_DIRS ${WAMR_ROOT_DIR}/core/iwasm/include)
    list(APPEND WASM_VM_LIBRARIES vmlib)
    
    add_definitions(-DCMSCRIPT_WAMR_ENABLED=1)
endif()

# WasmEdge Backend
if(CMSCRIPT_ENABLE_WASMEDGE)
    message(STATUS "Building with WasmEdge backend")
    
    # Configure WasmEdge
    set(WASMEDGE_BUILD_TESTS OFF CACHE BOOL "Build WasmEdge tests" FORCE)
    set(WASMEDGE_BUILD_TOOLS OFF CACHE BOOL "Build WasmEdge tools" FORCE)
    set(WASMEDGE_BUILD_AOT_RUNTIME OFF CACHE BOOL "Build WasmEdge AOT runtime" FORCE)
    set(WASMEDGE_BUILD_SHARED_LIB ON CACHE BOOL "Build WasmEdge shared library" FORCE)
    
    # Add WasmEdge as subdirectory
    set(WASMEDGE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/WasmEdge)
    add_subdirectory(${WASMEDGE_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/wasmedge EXCLUDE_FROM_ALL)
    
    set(WASMEDGE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/WasmEdge/WasmEdgeModule.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WasmEdge/WasmEdgeModule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WasmEdge/WasmEdgeContext.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WasmEdge/WasmEdgeContext.cpp
    )
    
    list(APPEND WASM_VM_SOURCES ${WASMEDGE_SOURCES})
    list(APPEND WASM_VM_INCLUDE_DIRS ${WASMEDGE_ROOT_DIR}/include)
    list(APPEND WASM_VM_LIBRARIES wasmedge)
    
    add_definitions(-DCMSCRIPT_WASMEDGE_ENABLED=1)
endif()

# Source groups for IDE
source_group("WasmVM\\Public" FILES ${WASM_VM_PUBLIC_HEADERS})
source_group("WasmVM\\Core" FILES ${WASM_VM_CORE_FILES})
if(CMSCRIPT_ENABLE_WAMR)
    source_group("WasmVM\\WAMR" FILES ${WAMR_SOURCES})
endif()
if(CMSCRIPT_ENABLE_WASMEDGE)
    source_group("WasmVM\\WasmEdge" FILES ${WASMEDGE_SOURCES})
endif()

# Export variables
set(WASM_VM_SOURCES ${WASM_VM_SOURCES} PARENT_SCOPE)
set(WASM_VM_INCLUDE_DIRS ${WASM_VM_INCLUDE_DIRS} PARENT_SCOPE)
set(WASM_VM_LIBRARIES ${WASM_VM_LIBRARIES} PARENT_SCOPE)
