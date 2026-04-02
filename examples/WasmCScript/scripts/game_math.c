/**
 * Math utilities for game script
 * These functions run inside the WASM virtual machine
 */

/**
 * Add two integers
 * @param a First number
 * @param b Second number
 * @return Sum of a and b
 */
int script_math_add(int a, int b) {
    return a + b;
}

/**
 * Multiply two integers
 * @param a First number
 * @param b Second number
 * @return Product of a and b
 */
int script_math_multiply(int a, int b) {
    return a * b;
}

/**
 * Absolute value
 * @param x Input value
 * @return Absolute value of x
 */
int script_math_abs(int x) {
    return x < 0 ? -x : x;
}

/**
 * Simple pseudo-random number
 * @param seed Random seed
 * @return Random-ish number
 */
unsigned int script_math_rand(unsigned int seed) {
    /* Linear congruential generator (simple) */
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

/**
 * Clamp value between min and max
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
int script_math_clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
