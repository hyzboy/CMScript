#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Reflection Code Generator (MVP)

Input: C++ header files with H_CLASS / H_STRUCT / H_PROPERTY / H_FUNCTION annotations
Implementation: Invoke clang++ to parse AST (JSON) and generate native symbol registration skeleton code

Current version output:
- Reflection manifest (classes/structs, properties, methods)
- Registration skeleton (placeholder function names for later wrapper integration)
Design: Generalized for any script engine (WAMR, Lua, etc.)
"""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, List
import re


ANNOT_CLASS = "h_class"
ANNOT_STRUCT = "h_struct"
ANNOT_PROPERTY = "h_property"
ANNOT_FUNCTION = "h_function"


@dataclass
class MethodInfo:
    name: str


@dataclass
class PropertyInfo:
    name: str


@dataclass
class TypeInfo:
    kind: str  # class / struct
    name: str
    properties: List[PropertyInfo] = field(default_factory=list)
    methods: List[MethodInfo] = field(default_factory=list)


def run_clang_ast(input_file: pathlib.Path, clangxx: str, std: str, include_dirs: List[str]) -> Dict[str, Any]:
    cmd = [
        clangxx,
        f"-std={std}",
        "-Xclang",
        "-ast-dump=json",
        "-fsyntax-only",
        str(input_file),
    ]
    for inc in include_dirs:
        cmd.extend(["-I", inc])

    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.returncode != 0:
        raise RuntimeError(
            "clang AST parsing failed:\n"
            f"command: {' '.join(cmd)}\n\n"
            f"stderr:\n{proc.stderr}"
        )

    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as e:
        raise RuntimeError(f"clang output is not valid JSON: {e}") from e


def get_annotation_values(node: Dict[str, Any]) -> List[str]:
    values: List[str] = []
    for child in node.get("inner", []):
        if child.get("kind") == "AnnotateAttr":
            ann = child.get("annotation")
            if isinstance(ann, str):
                values.append(ann)
    return values


def has_annotation(node: Dict[str, Any], expected: str) -> bool:
    return expected in get_annotation_values(node)


def walk(node: Dict[str, Any]):
    yield node
    for child in node.get("inner", []):
        if isinstance(child, dict):
            yield from walk(child)


def parse_reflect_types(ast: Dict[str, Any]) -> List[TypeInfo]:
    out: List[TypeInfo] = []

    for node in walk(ast):
        if node.get("kind") not in ("CXXRecordDecl", "RecordDecl"):
            continue

        # Keep only definitions with names
        name = node.get("name")
        if not name:
            continue

        ann_values = set(get_annotation_values(node))
        if ANNOT_CLASS in ann_values:
            kind = "class"
        elif ANNOT_STRUCT in ann_values:
            kind = "struct"
        else:
            continue

        t = TypeInfo(kind=kind, name=name)

        for child in node.get("inner", []):
            ck = child.get("kind")

            if ck == "FieldDecl" and has_annotation(child, ANNOT_PROPERTY):
                pname = child.get("name")
                if pname:
                    t.properties.append(PropertyInfo(name=pname))

            if ck in ("CXXMethodDecl", "FunctionDecl") and has_annotation(child, ANNOT_FUNCTION):
                mname = child.get("name")
                if mname:
                    t.methods.append(MethodInfo(name=mname))


        out.append(t)

    return out


def sanitize_symbol(name: str) -> str:
    return re.sub(r"[^0-9a-zA-Z_]", "_", name)


def gen_header(types: List[TypeInfo], input_header: str, register_func_name: str) -> str:
    """Generate reflection header file."""
    lines: List[str] = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("#include \"wasm_export.h\"")
    lines.append("")
    lines.append("// Auto-generated file, do not edit")
    lines.append(f"// Source: {input_header}")
    lines.append("")
    lines.append(f"bool {register_func_name}();")
    lines.append("")
    lines.append("// Reflection manifest")
    for t in types:
        lines.append(f"// - {t.kind} {t.name}")
        for p in t.properties:
            lines.append(f"//   property: {p.name}")
        for m in t.methods:
            lines.append(f"//   function: {m.name}")
    lines.append("")
    return "\n".join(lines)


def gen_cpp(types: List[TypeInfo], header_name: str, register_func_name: str) -> str:
    """Generate reflection implementation file with native symbol registration."""
    lines: List[str] = []
    lines.append(f"#include \"{header_name}\"")
    lines.append("")
    lines.append("// Auto-generated file, do not edit")
    lines.append("")

    # Generate placeholder wrapper declarations; can be extended with real parameter unpacking later
    for t in types:
        for m in t.methods:
            wrapper = f"h_reflect_wrap__{sanitize_symbol(t.name)}__{sanitize_symbol(m.name)}"
            lines.append(f"static void {wrapper}(wasm_exec_env_t exec_env) {{")
            lines.append("    (void)exec_env;")
            lines.append("    // TODO: retrieve object handle + unpack parameters + call real method")
            lines.append("}")
            lines.append("")

    lines.append(f"bool {register_func_name}() {{")
    lines.append("    static NativeSymbol native_symbols[] = {")

    for t in types:
        for m in t.methods:
            wrapper = f"h_reflect_wrap__{sanitize_symbol(t.name)}__{sanitize_symbol(m.name)}"
            export_name = f"{t.name}.{m.name}"
            lines.append(f"        {{ \"{export_name}\", (void*){wrapper}, NULL, NULL }},")

    lines.append("    };")
    lines.append("")
    lines.append("    const uint32_t n = (uint32_t)(sizeof(native_symbols) / sizeof(native_symbols[0]));")
    lines.append("    // Register to env module by convention")
    lines.append("    return wasm_runtime_register_natives(\"env\", native_symbols, n);")
    lines.append("}")
    lines.append("")

    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser(description="Reflection Code Generator (H_CLASS/H_STRUCT/H_FUNCTION/H_PROPERTY)")
    ap.add_argument("--input", required=True, help="input header file")
    ap.add_argument("--out-h", required=True, help="output header file")
    ap.add_argument("--out-cpp", required=True, help="output cpp file")
    ap.add_argument("--register-func", default="", help="generated registration function name")
    ap.add_argument("--clangxx", default="clang++", help="clang++ executable")
    ap.add_argument("--std", default="c++20", help="C++ standard")
    ap.add_argument("-I", dest="includes", action="append", default=[], help="additional include paths")
    args = ap.parse_args()

    in_file = pathlib.Path(args.input).resolve()
    out_h = pathlib.Path(args.out_h).resolve()
    out_cpp = pathlib.Path(args.out_cpp).resolve()
    register_func = args.register_func.strip()
    if not register_func:
        register_func = f"h_reflect_register_all_{sanitize_symbol(in_file.stem).lower()}"

    ast = run_clang_ast(in_file, args.clangxx, args.std, args.includes)
    types = parse_reflect_types(ast)

    out_h.parent.mkdir(parents=True, exist_ok=True)
    out_cpp.parent.mkdir(parents=True, exist_ok=True)

    header_text = gen_header(types, str(in_file), register_func)
    cpp_text = gen_cpp(types, out_h.name, register_func)

    out_h.write_text(header_text, encoding="utf-8")
    out_cpp.write_text(cpp_text, encoding="utf-8")

    print(f"[reflect_codegen] found {len(types)} reflected types")
    for t in types:
        print(f"  - {t.kind} {t.name}: {len(t.properties)} properties, {len(t.methods)} methods")
    print(f"[reflect_codegen] generated: {out_h}")
    print(f"[reflect_codegen] generated: {out_cpp}")
    print(f"[reflect_codegen] register function: {register_func}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as e:
        print(f"[reflect_codegen] error: {e}", file=sys.stderr)
        raise
