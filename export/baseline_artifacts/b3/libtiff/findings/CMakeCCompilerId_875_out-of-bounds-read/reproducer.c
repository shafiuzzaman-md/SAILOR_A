#include <stdio.h>
#include <stdlib.h>

/*
   Reproduce the CMakeCCompilerId.c bug:
   Indexing into a string using argc without bounds checking.

   We declare the strings as global arrays so AddressSanitizer places redzones
   around them, ensuring an out-of-bounds read is reliably detected.
*/

/* These mirror the identifiers used in the vulnerable file */
const char info_language_standard_default[]   = "INFO:standard_default[ON]";
const char info_language_extensions_default[] = "INFO:extensions_default[ON]";

/* Function that mimics the vulnerable access at line 875 */
static int cmake_id_like_main(int argc) {
    volatile int require = 0;
    /* Out-of-bounds read when argc >= sizeof(array) */
    require += info_language_standard_default[argc];
    require += info_language_extensions_default[argc];
    return require;
}

int main(void) {
    /* Craft an argc that is one past the end of info_language_standard_default */
    int crafted_argc = (int)sizeof(info_language_standard_default); /* includes NUL, so this is OOB by 1 */

    /* Trigger the vulnerable code path */
    int r = cmake_id_like_main(crafted_argc);

    /* Use the result to avoid any chance of it being optimized away */
    printf("result=%d\n", r);
    return 0;
}
