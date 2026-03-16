// Simple addition function for WebAssembly
// Compile with: scripts/compile_to_wasm.sh examples/wasm/add.c build/add.wasm

__attribute__((used))
__attribute__((export_name("add")))
int add(int a, int b) {
    return a + b;
}

__attribute__((used))
__attribute__((export_name("subtract")))
int subtract(int a, int b) {
    return a - b;
}

__attribute__((used))
__attribute__((export_name("multiply")))
int multiply(int a, int b) {
    return a * b;
}

__attribute__((used))
__attribute__((export_name("divide")))
int divide(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}
