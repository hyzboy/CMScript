#!/bin/bash
# Compile C/C++ source to WebAssembly (.wasm) using WASI-SDK
# Usage: compile_to_wasm.sh <input.c> <output.wasm> [additional_flags]

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input.c> <output.wasm> [additional_flags]"
    echo
    echo "Example:"
    echo "  $0 example.c example.wasm"
    echo "  $0 example.cpp example.wasm -O2"
    echo "  $0 lib.c lib.wasm \"-O2 -Wl,--export=myfunc\""
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="$2"
shift 2
EXTRA_FLAGS="$@"
WASI_SDK_DIR="../third_party/wasi"

# Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "ERROR: Input file not found: $INPUT_FILE"
    exit 1
fi

# Check if WASI-SDK is installed
if [ ! -f "$WASI_SDK_DIR/bin/clang" ]; then
    echo "ERROR: WASI-SDK not found at $WASI_SDK_DIR"
    echo "Please run: scripts/download_wasi_sdk.sh"
    exit 1
fi

# Determine if it's C or C++ based on extension
EXT="${INPUT_FILE##*.}"
COMPILER="clang"
case "$EXT" in
    cpp|cxx|cc|C)
        COMPILER="clang++"
        ;;
esac

echo "========================================"
echo "Compiling to WebAssembly"
echo "========================================"
echo "Input:    $INPUT_FILE"
echo "Output:   $OUTPUT_FILE"
echo "Compiler: $COMPILER"
echo "Flags:    $EXTRA_FLAGS"
echo "========================================"
echo

# Create output directory if needed
OUTPUT_DIR="$(dirname "$OUTPUT_FILE")"
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

# Compile
"$WASI_SDK_DIR/bin/$COMPILER" \
    -o "$OUTPUT_FILE" \
    "$INPUT_FILE" \
    $EXTRA_FLAGS

if [ $? -ne 0 ]; then
    echo
    echo "========================================"
    echo "Compilation FAILED!"
    echo "========================================"
    exit 1
fi

echo
echo "========================================"
echo "Compilation successful!"
echo "========================================"
echo "Output file: $OUTPUT_FILE"

# Show file size
if [ -f "$OUTPUT_FILE" ]; then
    FILE_SIZE=$(stat -f%z "$OUTPUT_FILE" 2>/dev/null || stat -c%s "$OUTPUT_FILE" 2>/dev/null)
    echo "File size: $FILE_SIZE bytes"
fi

echo
echo "Next steps:"
echo "1. Run with WAMR or WasmEdge runtime"
echo "2. Or compile to AOT: scripts/compile_to_aot.sh $OUTPUT_FILE output.aot"
echo

exit 0
