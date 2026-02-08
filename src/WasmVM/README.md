# WASM Virtual Machine for CMScript

This directory contains the WebAssembly virtual machine implementation for CMScript, providing a unified interface to multiple WASM runtimes.

## Supported Runtimes

### 1. WAMR (WebAssembly Micro Runtime)
- **Repository**: https://github.com/bytecodealliance/wasm-micro-runtime
- **Type**: Lightweight WASM runtime
- **Features**:
  - Fast interpreter
  - Small footprint
  - Suitable for embedded systems
  - AOT compilation support

### 2. WasmEdge
- **Repository**: https://github.com/WasmEdge/WasmEdge
- **Type**: High-performance WASM runtime
- **Features**:
  - WASI support
  - Extended features
  - Optimized for edge computing
  - Plugin system

## Architecture

The implementation provides a unified C++ interface that abstracts the underlying WASM runtime:

```
inc/hgl/wasm/
├── VM.h           - Common types and definitions
├── WasmVM.h       - Main VM initialization
├── WasmModule.h   - Module loading interface
└── WasmContext.h  - Execution context interface

src/WasmVM/
├── WasmVM.cpp     - Factory implementation
├── WAMR/          - WAMR-specific implementation
│   ├── WAMRModule.h/cpp
│   └── WAMRContext.h/cpp
└── WasmEdge/      - WasmEdge-specific implementation
    ├── WasmEdgeModule.h/cpp
    └── WasmEdgeContext.h/cpp
```

## Usage Example

```cpp
#include <hgl/wasm/WasmVM.h>

// Initialize VM
hgl::wasm::InitializeVM(hgl::wasm::VMType::WAMR);

// Create and load module
auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WAMR);
module->LoadFromFile("example.wasm");

// Create execution context
auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WAMR);
context->Instantiate(module);

// Call WASM function
hgl::wasm::Value args[2];
args[0] = hgl::wasm::Value::MakeI32(10);
args[1] = hgl::wasm::Value::MakeI32(32);

hgl::wasm::Value result[1];
context->CallFunction("add", args, 2, result, 1);

// Cleanup
hgl::wasm::CleanupVM(hgl::wasm::VMType::WAMR);
```

## Building

The WASM VM is built as part of CMScript. CMake options:

- `CMSCRIPT_ENABLE_WASM=ON/OFF` - Enable/disable WASM VM (default: ON)
- `CMSCRIPT_ENABLE_WAMR=ON/OFF` - Enable/disable WAMR backend (default: ON)
- `CMSCRIPT_ENABLE_WASMEDGE=ON/OFF` - Enable/disable WasmEdge backend (default: ON)

### Requirements

- CMake 3.20+
- C++20 compiler
- Git (for submodules)

### Build Commands

```bash
# Clone with submodules
git clone --recursive https://github.com/hyzboy/CMScript.git

# Or initialize submodules after clone
git submodule update --init --recursive

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

## Interface Design

### Module Loading
Both runtimes support:
- Loading from file: `LoadFromFile(filename)`
- Loading from memory: `LoadFromMemory(data, size)`

### Function Execution
Both runtimes support:
- Generic function calls with typed arguments
- Convenience methods for common patterns
- Memory access and manipulation

### Type System
Unified type system supporting:
- `ValueType::I32` - 32-bit integer
- `ValueType::I64` - 64-bit integer
- `ValueType::F32` - 32-bit float
- `ValueType::F64` - 64-bit float

## Examples

See `examples/` directory:
- `hello_wamr.cpp` - WAMR example
- `hello_wasmedge.cpp` - WasmEdge example
- `wasm/` - C/C++ source examples for compiling to WebAssembly

Both examples use the same WASM module demonstrating the unified interface.

## Compiling C/C++ to WebAssembly

To compile your own C/C++ code to WebAssembly modules:

### Quick Start

```bash
# 1. Download WASI-SDK
scripts/download_wasi_sdk.sh  # or .bat on Windows

# 2. Compile C/C++ to .wasm
scripts/compile_to_wasm.sh examples/wasm/add.c build/add.wasm

# 3. (Optional) Compile to AOT for better performance
scripts/compile_to_aot.sh build/add.wasm build/add.aot
```

### Detailed Guide

For comprehensive instructions on:
- Installing WASI-SDK
- Compiling C/C++ to .wasm format
- Compiling .wasm to .aot format
- Optimization options
- Troubleshooting

See the [WASI-SDK Usage Guide](../../docs/WASI_SDK_GUIDE.md).

## License

This implementation follows the licensing of:
- CMScript project
- WAMR (Apache-2.0 with LLVM exception)
- WasmEdge (Apache-2.0)
