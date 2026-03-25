#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
   This reproducer emulates the relevant pieces of the CMakeCCompilerId.c
   program where it indexes several info_* strings with argc without
   bounds checks. We construct these globals so that only info_platform
   will be too short for the chosen argc, triggering an out-of-bounds read
   at that access.
*/

/* Globals analogous to those in CMakeCCompilerId.c */
const char* info_compiler;
const char* info_platform;
const char* info_arch;
const char* info_version;
const char* info_version_internal;
const char* info_simulate;
const char* info_simulate_version;
const char* info_cray;
const char* info_language_standard_default;
const char* info_language_extensions_default;

/* Vulnerable function logic extracted/renamed from CMakeCCompilerId.c */
static int cmake_id_main(int argc, char* argv[]) {
    int require = 0;
    /* The following lines mirror the pattern from the original file */
    require += info_compiler[argc];            /* in-bounds for our crafted argc */
    require += info_platform[argc];            /* out-of-bounds read here */
    require += info_arch[argc];
    require += info_language_standard_default[argc];
    require += info_language_extensions_default[argc];
    (void)argv;
    return require;
}

int main(void) {
    /*
      Prepare the globals so that:
        - info_compiler is large enough (no OOB on first access)
        - info_platform is very small (OOB on second access)
        - the rest are large enough
    */

    /* Large buffer for info_compiler (size 128) */
    char* buf_compiler = (char*)malloc(128);
    memset(buf_compiler, 'A', 127);
    buf_compiler[127] = '\0';
    info_compiler = buf_compiler;

    /* Small buffer for info_platform (size 4, i.e., valid indices 0..3) */
    char* buf_platform = (char*)malloc(4);
    /* "PLT" + NUL */
    buf_platform[0] = 'P';
    buf_platform[1] = 'L';
    buf_platform[2] = 'T';
    buf_platform[3] = '\0';
    info_platform = buf_platform;

    /* Other buffers made sufficiently large */
    char* buf_arch = (char*)malloc(64);
    memset(buf_arch, 'B', 63);
    buf_arch[63] = '\0';
    info_arch = buf_arch;

    char* buf_lang_std = (char*)malloc(64);
    snprintf(buf_lang_std, 64, "INFO:standard_default[99]");
    info_language_standard_default = buf_lang_std;

    char* buf_lang_ext = (char*)malloc(64);
    snprintf(buf_lang_ext, 64, "INFO:extensions_default[ON]");
    info_language_extensions_default = buf_lang_ext;

    /* Unused in our trimmed function but declare to mirror structure */
    info_version = "INFO:version[unused]";
    info_version_internal = "INFO:version_internal[unused]";
    info_simulate = "INFO:simulate[unused]";
    info_simulate_version = "INFO:simulate_version[unused]";
    info_cray = "INFO:cray[unused]";

    /* Choose argc to be within bounds for info_compiler but out-of-bounds for info_platform.
       info_platform buffer size is 4, valid indices are 0..3. Using argc = 4 triggers OOB. */
    int crafted_argc = 4;

    /* Call the vulnerable logic. AddressSanitizer should report an out-of-bounds read
       (heap-buffer-overflow) at the info_platform[argc] access. */
    int ret = cmake_id_main(crafted_argc, NULL);

    /* Prevent compiler from optimizing everything away (even though -O0 is used) */
    fprintf(stderr, "Returned: %d\n", ret);

    /* Clean up (not reached if ASan aborts, but harmless) */
    free(buf_compiler);
    free(buf_platform);
    free(buf_arch);
    free(buf_lang_std);
    free(buf_lang_ext);

    return 0;
}
