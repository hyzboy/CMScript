# DevilScript Roadmap (Class-C Syntax + Modern VM)

This document is a concrete, phased checklist for evolving DevilScript into a class-C syntax language with a modern VM design.

## Phase 0: Goals and Constraints
- Define the minimal language subset (vars/expr/functions/if/while/return).
- Decide on optional constructs: for/switch/struct/class.
- Define the type system (bool/int/float/string; decide on int64/double).
- Define memory model boundaries between script objects and C++ objects.

## Phase 1: Lexer/Parser Upgrade (AST)
- Write the formal grammar (EBNF or similar).
- Replace parse-and-execute with AST generation.
- AST node list:
  - Literal
  - Identifier
  - Binary/Unary
  - Call
  - VarDecl
  - If
  - While
  - Return
  - Block
- Unified diagnostics with line/column, error codes, and source snippets.

## Phase 2: Bytecode and Compiler
- Design a stable instruction set (start with stack-based VM).
- Build constant pools (numbers, strings, function table).
- Compile AST to bytecode.
- Define function/local/parameter tables.
- Add simple optimizations (constant fold, dead code elimination).

## Phase 3: VM Runtime
- Define stack frame layout (return address, locals, temporaries).
- Define calling convention (arg order, return value rules).
- Implement builtin ops (arith/compare/logic).
- Add runtime error handling (safe abort, error messages).

## Phase 4: Type System and Native Binding
- Central type registry (script types <-> C++ types).
- Rules for implicit/explicit conversions.
- Unified binding API (typed MapFunc is the baseline).
- Decide policy for const/ref/ptr parameters (start with value-only).

## Phase 5: Memory Management
- Choose a GC model (ref-count or mark-and-sweep).
- Script object lifetime independent from C++ lifetime.
- Unified memory model for string/array/object.

## Phase 6: Standard Library and Modules
- Minimal stdlib: string/array/map/math.
- Module system (import, namespaces, isolation).
- Source file management and compilation cache.

## Phase 7: Debugging and Tooling
- Breakpoints, step, variable inspection.
- Debug info mapping (line -> bytecode PC).
- Stack trace with function names and source lines.

## Phase 8: Performance and Extensibility
- Peephole optimizations and instruction-level tuning.
- Optional JIT or multithreaded execution.
- Hot reload / script caching.

## Suggested Deliverables per Phase
- Phase 1: AST classes, parser tests, diagnostic output.
- Phase 2: Bytecode spec and AST->bytecode compiler.
- Phase 3: VM loop with call stack, error path.
- Phase 4: Type registry and typed binding helpers.
- Phase 5: GC prototype and memory ownership rules.
- Phase 6: stdlib and module import.
- Phase 7: Debugger hooks and stack trace.
- Phase 8: Optimization passes and perf benchmarks.
