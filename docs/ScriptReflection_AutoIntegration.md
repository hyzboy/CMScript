# Script Reflection Auto-Integration (Large Project Pattern)

## Goal

Provide a robust pattern so generated reflection registration code is always built and can be reliably registered at runtime in large projects.

This document uses the WasmCScript example as a concrete implementation.

## Key Problems in Large Projects

1. Generated files are easy to forget adding to build targets.
2. A single fixed registration function name can collide across multiple modules.
3. Pure static initialization may fail silently if translation units are dropped by the linker.
4. Teams need clear explicit control and optional auto-registration behavior.

## Adopted Solution

A mixed strategy is used:

1. Build-time auto generation with CMake `add_custom_command`.
2. Per-module unique register function names from codegen.
3. Explicit bootstrap entry point for deterministic registration.
4. Optional static auto-registration behind a compile definition.

## Files and Roles

- `examples/WasmCScript/CMakeLists.txt`
  - Runs codegen automatically.
  - Adds generated `.cpp` into `WasmCScriptHost` target.
- `examples/WasmCScript/host/ReflectionBootstrap.h`
  - Public explicit runtime registration entry.
- `examples/WasmCScript/host/ReflectionBootstrap.cpp`
  - Calls generated registration function.
  - Supports optional static auto-registration via `H_REFLECT_AUTO_REGISTER`.
- `src/WasmVM/tools/wamr_codegen.py`
  - Parses `H_CLASS/H_STRUCT/H_PROPERTY/H_FUNCTION` clang annotations.
  - Supports `--register-func` for unique function naming.

## Build-Time Pipeline

```
GameHost.h (H_* annotations)
  -> clang AST JSON (inside codegen)
  -> GameHost.reflect.h / GameHost.reflect.cpp
  -> compile into WasmCScriptHost library
```

### CMake Pattern

Use custom command outputs as first-class build artifacts:

```cmake
add_custom_command(
    OUTPUT <generated.h> <generated.cpp>
    COMMAND python wamr_codegen.py ... --register-func h_reflect_register_gamehost
    DEPENDS <input-header> <codegen-script>
)

add_library(WasmCScriptHost STATIC
    host/GameHost.cpp
    host/ReflectionBootstrap.cpp
    <generated.cpp>
)
```

This guarantees generated source is compiled into the target every time inputs change.

## Runtime Registration Pattern

### Recommended (Explicit)

Call this once during VM startup:

```cpp
#include "ReflectionBootstrap.h"

if (!register_all_script_bindings()) {
    // fail fast or log and abort script subsystem init
}
```

Why this is recommended:

1. Clear call site and lifecycle control.
2. Deterministic error handling.
3. Easy unit/integration testing.

### Optional (Implicit)

If `H_REFLECT_AUTO_REGISTER` is defined, a static initializer in `ReflectionBootstrap.cpp` triggers registration automatically.

Use this only when your engine startup policy accepts implicit side effects.

## Codegen Naming Strategy

`wamr_codegen.py` now supports:

```text
--register-func <name>
```

If not provided, it auto-derives:

```text
h_reflect_register_all_<header_stem>
```

This avoids collisions in multi-module builds.

## Annotation Matching Strategy

The code generator now matches exact annotation values:

- `h_class`
- `h_struct`
- `h_property`
- `h_function`

This prevents accidental pickup of unrelated clang annotations.

## Scaling to Multi-Module Projects

For N script-exposed modules, generate N registration units and aggregate them:

```cpp
bool register_all_script_bindings() {
    bool ok = true;
    ok = h_reflect_register_gamehost() && ok;
    ok = h_reflect_register_uimodule() && ok;
    ok = h_reflect_register_physics() && ok;
    return ok;
}
```

Then call only this single aggregation function from startup.

## CI Recommendations

1. Add a configure/build job that verifies generated files are recreated from clean build directories.
2. Fail CI if codegen changes are needed but not committed (for workflows that commit generated files).
3. Add a smoke test that calls `register_all_script_bindings()` and asserts success.

## Operational Recommendations

1. Keep generated files in build tree for normal builds.
2. Use explicit registration in production initialization code.
3. Keep implicit static registration as opt-in for quick experiments only.
4. Standardize one bootstrap function per subsystem.

## Example Commands

```powershell
cmake -S examples/WasmCScript -B build/wasmcscript
cmake --build build/wasmcscript --config Release
```

Manual codegen run (debugging only):

```powershell
python examples/WasmCScript/run_codegen.py
```
