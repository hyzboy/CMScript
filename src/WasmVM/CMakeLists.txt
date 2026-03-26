cmake_minimum_required(VERSION 3.20)

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
message(STATUS "Building with WAMR backend")

# Configure WAMR
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 1)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_MULTI_MODULE 0)
set(WAMR_BUILD_LIB_PTHREAD 0)
set(WAMR_BUILD_MINI_LOADER 0)
set(WAMR_BUILD_SIMD 0)
set(WAMR_BUILD_REF_TYPES 0)

# Add WAMR as subdirectory
set(WAMR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/wasm-micro-runtime)

# Set WAMR platform based on system
if(WIN32)
    set(WAMR_BUILD_PLATFORM "windows")
elseif(APPLE)
    set(WAMR_BUILD_PLATFORM "darwin")
else()
    set(WAMR_BUILD_PLATFORM "linux")
endif()

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

# Source groups for IDE
source_group("WasmVM\\Public" FILES ${WASM_VM_PUBLIC_HEADERS})
source_group("WasmVM\\Core" FILES ${WASM_VM_CORE_FILES})
source_group("WasmVM\\WAMR" FILES ${WAMR_SOURCES})

# Export variables
set(WASM_VM_SOURCES ${WASM_VM_SOURCES} PARENT_SCOPE)
set(WASM_VM_INCLUDE_DIRS ${WASM_VM_INCLUDE_DIRS} PARENT_SCOPE)
set(WASM_VM_LIBRARIES ${WASM_VM_LIBRARIES} PARENT_SCOPE)
