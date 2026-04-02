#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Runner script for reflection code generation on host interfaces

This script demonstrates how to use the codegen tool to generate reflection
code for the example GameHost.h header file.

Usage:
    python run_codegen.py
    
The script will:
1. Locate the codegen tool
2. Parse GameHost.h for H_* macro annotations
3. Generate reflection header and implementation files in ./gen/
"""

import subprocess
import pathlib
import sys

def run_codegen():
    """Run the code generator on GameHost.h"""
    
    # Get script directory
    script_dir = pathlib.Path(__file__).parent.resolve()
    
    # Locate codegen tool
    tools_dir = script_dir.parent.parent / "src" / "WasmVM" / "tools"
    codegen_tool = tools_dir / "wamr_codegen.py"
    
    if not codegen_tool.exists():
        print(f"[error] wamr_codegen.py not found at {codegen_tool}", file=sys.stderr)
        return False
    
    # Input header (host interface)
    input_header = script_dir / "host" / "GameHost.h"
    if not input_header.exists():
        print(f"[error] input header not found at {input_header}", file=sys.stderr)
        return False
    
    # Output directory
    gen_dir = script_dir / "host" / "gen"
    gen_dir.mkdir(parents=True, exist_ok=True)
    
    # Output files
    out_h = gen_dir / "GameHost.reflect.h"
    out_cpp = gen_dir / "GameHost.reflect.cpp"
    
    # Include directories
    # 1. host directory (for local includes)
    # 2. ULRE include directory (for hgl/Reflect.h)
    include_dirs = [
        str(script_dir / "host"),
        str(script_dir.parent.parent / "inc"),
    ]
    
    # Build command
    cmd = [
        sys.executable,
        str(codegen_tool),
        "--input", str(input_header),
        "--out-h", str(out_h),
        "--out-cpp", str(out_cpp),
        "--register-func", "h_reflect_register_gamehost",
        "--std", "c++20",
    ]
    
    # Add include directories
    for inc_dir in include_dirs:
        cmd.extend(["-I", inc_dir])
    
    print(f"[codegen] Running: {' '.join(cmd)}")
    print()
    
    # Run code generator
    result = subprocess.run(cmd, text=True)
    
    if result.returncode == 0:
        print()
        print("[codegen] ✓ Code generation succeeded")
        print(f"[codegen]   Generated: {out_h}")
        print(f"[codegen]   Generated: {out_cpp}")
        print()
        print("Next steps:")
        print("  1. Include generated header in your host application")
        print("  2. Link generated .cpp with your host code")
        print("  3. Call wamr_auto_register_all() during VM initialization")
        return True
    else:
        print(f"[codegen] ✗ Code generation failed (exit code: {result.returncode})")
        return False


if __name__ == "__main__":
    success = run_codegen()
    sys.exit(0 if success else 1)
