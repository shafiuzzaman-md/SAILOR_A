#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mimic the CMakeCCompilerId.c globals as string literals accessed via const char* */
static const char info_compiler_lit[] = "INFO:compiler[clang]";
static const char* info_compiler = info_compiler_lit;
static const char info_platform_lit[] = "INFO:platform[linux]";
static const char* info_platform = info_platform_lit;
static const char info_arch_lit[] = "INFO:arch[x86_64]";
static const char* info_arch = info_arch_lit;
static const char info_language_standard_default_lit[] = "INFO:standard_default[11]";
static const char* info_language_standard_default = info_language_standard_default_lit;
static const char info_language_extensions_default_lit[] = "INFO:extensions_default[ON]";
static const char* info_language_extensions_default = info_language_extensions_default_lit;

/* Vulnerable code path modeled after CMakeCCompilerId.c:main */
static int cmake_compilerid_main_like(int argc, char** argv) {
    int require = 0;
    require += info_compiler[argc];
    require += info_platform[argc];
    require += info_arch[argc]; /* out-of-bounds when argc > strlen(info_arch_lit) */
    require += info_language_standard_default[argc];
    require += info_language_extensions_default[argc];
    (void)argv;
    return require;
}

int main(void) {
    /* Craft argc to be just past the end of info_arch to trigger OOB read */
    int crafted_argc = (int)strlen(info_arch_lit) + 1;
    char* dummy_argv[] = { (char*)"repro", NULL };

    volatile int res = cmake_compilerid_main_like(crafted_argc, dummy_argv);
    /* Use res so the compiler can't optimize away the reads */
    printf("Result: %d\n", res);
    return 0;
}
