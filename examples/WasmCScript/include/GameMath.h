#pragma once

/**
 * Simple C math functions for WASM
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add two integers
 * @param a First number
 * @param b Second number
 * @return The sum of a and b
 */
int math_add(int a, int b);

/**
 * Multiply two floating-point numbers
 * @param a First number
 * @param b Second number
 * @return The product of a and b
 */
float math_multiply(float a, float b);

/**
 * Calculate the absolute value
 * @param x Input value
 * @return Absolute value of x
 */
int math_abs(int x);

#ifdef __cplusplus
}
#endif
