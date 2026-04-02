cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# 核心配置
set(WAMR_BUILD_INTERP 1)           # 解释器模式（必须）
set(WAMR_BUILD_FAST_INTERP 1)      # 快速解释器（性能优化）
set(WAMR_BUILD_AOT 0)              # 关闭 AOT（游戏脚本不需要）
set(WAMR_BUILD_JIT 0)              # 关闭 JIT
set(WAMR_BUILD_LIBC_BUILTIN 1)     # 内置 libc（基本函数：malloc、memcpy等）
set(WAMR_BUILD_LIBC_WASI 0)        # 关闭 WASI（不需要文件/网络系统调用）

# 可选特性
set(WAMR_BUILD_SIMD 1)             # 启用 SIMD（向量运算，适合游戏计算）
set(WAMR_BUILD_REF_TYPES 1)        # 引用类型
set(WAMR_BUILD_MULTI_MODULE 0)     # 单模块模式（游戏脚本通常单文件）
set(WAMR_BUILD_LIB_PTHREAD 0)
set(WAMR_BUILD_MINI_LOADER 0)

# Add WAMR as subdirectory
set(WAMR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasm-micro-runtime)

# Set WAMR platform based on system
if(WIN32)
    set(WAMR_BUILD_PLATFORM "windows")
elseif(APPLE)
    set(WAMR_BUILD_PLATFORM "darwin")
else()
    set(WAMR_BUILD_PLATFORM "linux")
endif()

set(WAMR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/WAMRModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WAMRModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WAMRContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WAMRContext.cpp
)

list(APPEND WASM_VM_SOURCES ${WAMR_SOURCES})
list(APPEND WASM_VM_LIBRARIES vmlib)

# Source groups for IDE
source_group("WasmVM\\Public" FILES ${WASM_VM_PUBLIC_HEADERS})
source_group("WasmVM\\Core" FILES ${WASM_VM_CORE_FILES})
source_group("WasmVM\\WAMR" FILES ${WAMR_SOURCES})

add_subdirectory(wasm-micro-runtime)

# Build a standalone static library target for the WASM VM implementation
add_library(wasmvm STATIC ${WASM_VM_SOURCES})
target_include_directories(wasmvm PUBLIC ${WASM_VM_INCLUDE_DIRS})
target_link_libraries(wasmvm PUBLIC ${WASM_VM_LIBRARIES})
set_target_properties(wasmvm PROPERTIES
    OUTPUT_NAME "WasmVM"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
# Namespaced alias for downstream use
add_library(CMScript::WasmVM ALIAS wasmvm)

# Export variables
set(WASM_VM_SOURCES ${WASM_VM_SOURCES} PARENT_SCOPE)
set(WASM_VM_INCLUDE_DIRS ${WASM_VM_INCLUDE_DIRS} PARENT_SCOPE)
set(WASM_VM_LIBRARIES ${WASM_VM_LIBRARIES} PARENT_SCOPE)
