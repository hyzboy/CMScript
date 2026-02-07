# DevilScript Goals and Constraints (Current Baseline)

This document captures goals and constraints based on the current DevilScript codebase, to guide the next phase of language and VM evolution.

## Current Baseline (Facts)
- Script parser uses AngelScript tokenizer (as_tokenizer/as_tokendef).
- Script functions are parsed but currently do not accept parameters.
- Runtime executes a command list (VM commands) rather than a bytecode interpreter.
- Native binding uses a typed MapFunc API and a FuncMap table.
- Supported binding types are: bool, int, int8, int16, uint, uint8, uint16, float, string (char*).
- 64-bit types and double are present in tokens but not enabled in parsing/binding paths.
- Native calls are implemented by platform-specific call stubs (x86_32 and x86_64).

## Goals (Near-Term)
- Keep existing script behavior working while introducing new compiler/VM layers.
- Maintain a simple and explicit C-like syntax for core constructs.
- Preserve current native binding semantics and extend types in a controlled way.
- Improve diagnostics (line/column, clear error reasons).
- Enable a path to AST -> bytecode without breaking current scripts.

## Goals (Mid-Term)
- Move from parse-and-execute to AST and bytecode execution.
- Add script function parameters and local variable scoping rules.
- Introduce a minimal standard library (string/array/map/math).
- Provide a stable module/import mechanism.

## Constraints (Technical)
- The current VM is command-list based; any new bytecode VM must coexist or provide a migration path.
- Native call ABI is platform-specific and must keep working on x86_64 (and x86_32 if still required).
- Script string arguments are passed as char*; std::string is not supported in bindings.
- The parser is currently single-pass and intertwined with runtime structures.

## Constraints (Product/Compatibility)
- Existing scripts and binding code should continue to run during transition.
- Any new syntax should be additive or gated by versioning.
- The minimal runtime footprint should remain small (no mandatory heavy dependencies).

## Open Decisions
- Whether to keep stack-based execution or move to a register-based VM.
- How to implement memory management for script-owned objects (RC vs GC).
- The scope of C-like syntax (structs/classes, operator overloading, etc.).
- How to expose debugging facilities (breakpoints, step, stack trace).
