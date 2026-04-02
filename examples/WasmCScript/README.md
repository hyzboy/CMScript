# WAMR Script + Native Integration Example

This example demonstrates a complete host + script workflow for WAMR, including:

- H_* macro based host API reflection
- single-file code generation for quick iteration
- batch scan + CMake auto-integration for larger projects
- runtime registration aggregation

## Architecture Overview

```text
┌──────────────────────────────────────┐
│ Host Application (C/C++)            │
│ • GameHost / InputHandler / Audio   │
│ • Generated reflection wrappers      │
│ • Aggregated registration entry      │
└──────────────────────────────────────┘
                ↕ native calls
         wasm_runtime_register_natives()
                ↕
┌──────────────────────────────────────┐
│ WASM Runtime                         │
│ • Executes game_logic.wasm           │
│ • Executes game_math.wasm            │
│ • Calls exported native bindings     │
└──────────────────────────────────────┘
```

## Directory Structure

```text
WasmCScript/
├── README.md
├── CMakeLists.txt
├── run_codegen.py
├── scripts/
│   ├── game_logic.c
│   └── game_math.c
└── host/
    ├── GameHost.h
    └── GameHost.cpp
```

Generated files appear in different locations depending on workflow:

- Manual run_codegen.py flow: host/gen/*.reflect.h/.cpp

- CMake batch flow: build/reflect_gen/*.reflect.h/.cpp + ReflectionBootstrap.gen.cpp/.h

## Macro Annotations

Use macros from hgl/Reflect.h:

```cpp
class H_CLASS GameHost {
public:
    H_FUNCTION void init();
    H_FUNCTION uint32_t get_fps() const;

private:
    H_PROPERTY uint32_t fps;
};
```

Meaning:

- H_CLASS / H_STRUCT: mark reflectable type
- H_FUNCTION: mark script-callable method
- H_PROPERTY: mark reflectable field

These macros expand to clang annotate attributes (h_class, h_struct, h_function, h_property), and become no-ops on non-clang compilers.

## Full Operation Flow (Recommended)

### 1. Define host API

In host headers, mark types and members with H_* macros.

### 2. Implement host logic

Implement the real methods in .cpp files (for example GameHost.cpp).

### 3. Let CMake batch-scan and generate bindings

The project now uses a batch toolchain:

- src/WasmVM/tools/reflect_scan.py
- src/WasmVM/tools/ReflectScan.cmake
- src/WasmVM/tools/wamr_codegen.py

The WasmCScript CMakeLists.txt calls reflect_scan_directory(...), which:

1. scans the configured directory recursively for annotated files
2. generates one *.reflect.h/*.reflect.cpp per matched file
3. generates aggregated ReflectionBootstrap.gen.cpp/.h
4. builds a static library target WasmCScriptHost containing all generated reflection sources

### 4. Build

```bash
cmake -S CMScript/examples/WasmCScript -B build/wasmcscript
cmake --build build/wasmcscript --config Release
```

### 5. Runtime registration

Call the aggregated registration function once during VM startup:

```cpp
extern bool register_all_script_bindings();

if (!register_all_script_bindings()) {
    // handle registration failure
}
```

This function is generated in ReflectionBootstrap.gen.cpp and invokes every per-file generated register function.

### 6. Compile script side and run

```bash
WasmCompiler.bat scripts\game_logic.c -o game_logic.wasm
WasmCompiler.bat scripts\game_math.c -o game_math.wasm
```

Load wasm modules, instantiate, then call exported script entry points as usual.

## Quick Manual Flow (Single Header)

For fast local iteration on one header:

```bash
python run_codegen.py
```

This script generates:

- host/gen/GameHost.reflect.h
- host/gen/GameHost.reflect.cpp

By default, it passes:

- register function name: h_reflect_register_gamehost
- includes: host/, ../../inc

## Generated Code Naming

- Wrapper function prefix: h_reflect_wrap__Type__Method
- Per-file register function: h_reflect_register_<header_stem>
- Aggregated register function: register_all_script_bindings

Example native symbol export names remain Type.Method (for example GameHost.init).

## Add New Host Module Checklist

1. Add a new header/cpp under the scanned host directory.
2. Annotate types/methods/properties with H_* macros.
3. Re-run CMake configure (or rebuild if already configured).
4. Verify new *.reflect.cpp is generated under build/reflect_gen.
5. No manual bootstrap editing is required.

## Troubleshooting

clang++ not found:

- Ensure clang++ is in PATH, or pass CLANGXX in reflect_scan_directory(...).

No generated files:

- Check that headers contain H_CLASS or H_STRUCT.
- Confirm scan directory and exclude patterns in CMake are correct.

Registration failed at runtime:

- Ensure register_all_script_bindings() is called before invoking script-side host calls.
- Verify generated library WasmCScriptHost is linked into your executable.

Generated wrappers are stubs:

- This is expected. Extend wrapper bodies for argument marshalling and object lookup according to your runtime design.
