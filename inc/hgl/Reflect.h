#pragma once

// Unified reflection annotation macros
// Design goal: Simple user-side syntax (H_CLASS/H_STRUCT/H_PROPERTY/H_FUNCTION)
// Implementation: Maps to clang annotations for AST-based code generation
// Generalized for any script engine (not limited to WAMR)

#if defined(__clang__)
  // Type annotation usage:
  //   struct H_STRUCT Foo { ... };
  //   class  H_CLASS  Bar { ... };
  // Uses GNU attributes to support class/struct declaration positions.
  #define H_CLASS    __attribute__((annotate("h_class")))
  #define H_STRUCT   __attribute__((annotate("h_struct")))

  // Member annotations use standard attribute syntax (placed before declaration)
  #define H_PROPERTY [[clang::annotate("h_property")]]
  #define H_FUNCTION [[clang::annotate("h_function")]]
#else
  // Non-clang compilers: macros become no-ops (graceful degradation)
  #define H_CLASS
  #define H_STRUCT
  #define H_PROPERTY
  #define H_FUNCTION
#endif
