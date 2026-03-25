#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal replicas of the strings used by CMake's CMakeCCompilerId.c */
static const char info_compiler[] = "Clang";
static const char info_platform[] = "Linux";
static const char info_arch[] = "x86_64";
static const char info_language_standard_default[] = "standard_default[17]";
static const char info_language_extensions_default[] = "extensions_default[ON]";

/*
 * This function mirrors the vulnerable indexing pattern from
 * build_bc/CMakeFiles/3.28.3/CompilerIdC/CMakeCCompilerId.c:main
 * It uses argc directly as an index into fixed-size string literals.
 */
static int cmake_id_like_main(int argc) {
    volatile int require = 0; /* volatile to prevent optimization */
    /* Out-of-bounds reads when argc exceeds the length of these strings */
    require += info_compiler[argc];
    require += info_platform[argc];
    require += info_arch[argc];
    require += info_language_standard_default[argc];
    require += info_language_extensions_default[argc];
    return require;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    /* Craft an argc much larger than any of the above string lengths */
    int fake_argc = 1000;

    int res = cmake_id_like_main(fake_argc);
    /* Use the result to avoid optimization removing the calls */
    printf("require=%d\n", res);
    return 0;
}
