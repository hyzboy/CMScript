// Math library functions for WebAssembly
// Compile with: scripts/compile_to_wasm.sh examples/wasm/math_lib.c build/math_lib.wasm -O2

#include <math.h>

__attribute__((used))
__attribute__((export_name("calculate_circle_area")))
double calculate_circle_area(double radius) {
    return 3.14159265359 * radius * radius;
}

__attribute__((used))
__attribute__((export_name("calculate_circle_circumference")))
double calculate_circle_circumference(double radius) {
    return 2.0 * 3.14159265359 * radius;
}

__attribute__((used))
__attribute__((export_name("calculate_hypotenuse")))
double calculate_hypotenuse(double a, double b) {
    return sqrt(a * a + b * b);
}

__attribute__((used))
__attribute__((export_name("factorial")))
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

__attribute__((used))
__attribute__((export_name("fibonacci")))
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

__attribute__((used))
__attribute__((export_name("power")))
double power(double base, int exponent) {
    return pow(base, exponent);
}

__attribute__((used))
__attribute__((export_name("square_root")))
double square_root(double x) {
    return sqrt(x);
}
