#!/bin/bash
# Compile WebAssembly (.wasm) to AOT format (.aot) using WAMR compiler
# Usage: compile_to_aot.sh <input.wasm> <output.aot> [additional_flags]

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input.wasm> <output.aot> [additional_flags]"
    echo
    echo "Example:"
    echo "  $0 example.wasm example.aot"
    echo "  $0 example.wasm example.aot --opt-level=3"
    echo "  $0 example.wasm example.aot \"--target=x86_64 --opt-level=3\""
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="$2"
shift 2
EXTRA_FLAGS="$@"
WAMR_DIR="../third_party/wasm-micro-runtime"
WAMRC_BIN="$WAMR_DIR/wamr-compiler/build/wamrc"

# Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "ERROR: Input file not found: $INPUT_FILE"
    exit 1
fi

# Check if wamrc exists
if [ ! -f "$WAMRC_BIN" ]; then
    echo "ERROR: wamrc compiler not found at $WAMRC_BIN"
    echo
    echo "Please build WAMR AOT compiler first:"
    echo "  1. Initialize submodules: git submodule update --init --recursive"
    echo "  2. cd $WAMR_DIR/wamr-compiler"
    echo "  3. mkdir build && cd build"
    echo "  4. cmake .."
    echo "  5. cmake --build ."
    echo
    exit 1
fi

echo "========================================"
echo "Compiling to AOT format"
echo "========================================"
echo "Input:  $INPUT_FILE"
echo "Output: $OUTPUT_FILE"
echo "Flags:  $EXTRA_FLAGS"
echo "========================================"
echo

# Create output directory if needed
OUTPUT_DIR="$(dirname "$OUTPUT_FILE")"
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
fi

# Compile
"$WAMRC_BIN" \
    -o "$OUTPUT_FILE" \
    $EXTRA_FLAGS \
    "$INPUT_FILE"

if [ $? -ne 0 ]; then
    echo
    echo "========================================"
    echo "AOT Compilation FAILED!"
    echo "========================================"
    exit 1
fi

echo
echo "========================================"
echo "AOT Compilation successful!"
echo "========================================"
echo "Output file: $OUTPUT_FILE"

# Show file size comparison
echo
echo "File size comparison:"
if [ -f "$INPUT_FILE" ]; then
    WASM_SIZE=$(stat -f%z "$INPUT_FILE" 2>/dev/null || stat -c%s "$INPUT_FILE" 2>/dev/null)
    echo "  .wasm: $WASM_SIZE bytes"
fi
if [ -f "$OUTPUT_FILE" ]; then
    AOT_SIZE=$(stat -f%z "$OUTPUT_FILE" 2>/dev/null || stat -c%s "$OUTPUT_FILE" 2>/dev/null)
    echo "  .aot:  $AOT_SIZE bytes"
fi

echo
echo "Next steps:"
echo "1. Load AOT module in WAMR runtime"
echo "2. Enjoy faster execution!"
echo

exit 0
