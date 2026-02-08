# CMScript

CMScript is a multi-backend scripting system supporting multiple virtual machine implementations.

## Features

### DevilVM
Custom scripting virtual machine with:
- Dynamic scripting language
- Native function mapping
- Context-based execution

### WASM VM (New!)
WebAssembly virtual machine support with unified interface:
- **WAMR** (WebAssembly Micro Runtime) - Lightweight runtime for embedded systems
- **WasmEdge** - High-performance runtime for edge computing
- Unified C++ API for both runtimes
- Support for standard WASM modules

## Project Structure

```
CMScript/
├── inc/hgl/          - Public headers
│   ├── devil/        - DevilVM headers
│   └── wasm/         - WASM VM headers
├── src/              - Source code
│   ├── DevilVM/      - DevilVM implementation
│   └── WasmVM/       - WASM VM implementation
├── examples/         - Example programs
└── third_party/      - External dependencies (submodules)
    ├── wasm-micro-runtime/  - WAMR
    └── WasmEdge/            - WasmEdge
```

## Building

### Requirements
- CMake 3.28+
- C++20 compiler
- Git

### Clone and Build

```bash
# Clone with submodules
git clone --recursive https://github.com/hyzboy/CMScript.git
cd CMScript

# Or initialize submodules after clone
git submodule update --init --recursive

# Configure and build
mkdir build && cd build
cmake ..
cmake --build .
```

### Build Options

- `CMSCRIPT_ENABLE_DEVIL=ON/OFF` - Enable DevilScript backend (default: ON)
- `CMSCRIPT_ENABLE_WASM=ON/OFF` - Enable WASM VM backend (default: ON)
- `CMSCRIPT_ENABLE_WAMR=ON/OFF` - Enable WAMR runtime (default: ON)
- `CMSCRIPT_ENABLE_WASMEDGE=ON/OFF` - Enable WasmEdge runtime (default: ON)

Example:
```bash
cmake -DCMSCRIPT_ENABLE_WAMR=ON -DCMSCRIPT_ENABLE_WASMEDGE=OFF ..
```

## Usage Examples

### DevilVM Example

```cpp
#include <hgl/devil/DevilVM.h>

hgl::devil::Module module;
module.MapFunc("print", &PrintFunction);
module.AddScript("func main(){ print(\"Hello!\"); }");

hgl::devil::Context context(&module);
context.Start("main");
```

### WASM VM Example

```cpp
#include <hgl/wasm/WasmVM.h>

// Initialize and use WAMR
hgl::wasm::InitializeVM(hgl::wasm::VMType::WAMR);
auto module = hgl::wasm::CreateWasmModule(hgl::wasm::VMType::WAMR);
module->LoadFromFile("example.wasm");

auto context = hgl::wasm::CreateWasmContext(hgl::wasm::VMType::WAMR);
context->Instantiate(module);
context->CallFunction("exported_function");

hgl::wasm::CleanupVM(hgl::wasm::VMType::WAMR);
```

See `examples/` directory for more examples.

## Documentation

- [WASM VM Documentation](src/WasmVM/README.md)

## License

This project follows the licensing of its components:
- CMScript code: (See LICENSE file)
- WAMR: Apache-2.0 with LLVM exception
- WasmEdge: Apache-2.0
