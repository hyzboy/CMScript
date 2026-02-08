// String manipulation functions for WebAssembly
// Compile with: scripts/compile_to_wasm.sh examples/wasm/string_ops.c build/string_ops.wasm

#include <string.h>

__attribute__((used))
__attribute__((export_name("string_length")))
int string_length(const char* str) {
    return strlen(str);
}

__attribute__((used))
__attribute__((export_name("string_compare")))
int string_compare(const char* str1, const char* str2) {
    return strcmp(str1, str2);
}

__attribute__((used))
__attribute__((export_name("string_copy")))
char* string_copy(char* dest, const char* src) {
    return strcpy(dest, src);
}

__attribute__((used))
__attribute__((export_name("string_concat")))
char* string_concat(char* dest, const char* src) {
    return strcat(dest, src);
}

__attribute__((used))
__attribute__((export_name("count_chars")))
int count_chars(const char* str, char ch) {
    int count = 0;
    for (const char* p = str; *p; p++) {
        if (*p == ch) count++;
    }
    return count;
}
