#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

// Force malloc() to fail inside the included vulnerable program
static void *failing_malloc(size_t size) {
    (void)size;
    return NULL; // Simulate allocation failure (OOM)
}

// Redirect malloc used in the included source to our failing allocator
#define malloc(SZ) failing_malloc(SZ)

// Rename main from the vulnerable program so we can call it from our own main
#define main tiff_palette_main

// Include the actual vulnerable program from the project
#include "/tmp/libtiff_upstream/contrib/dbs/tiff-palette.c"

// Restore names for our file
#undef main
#undef malloc

int main(void) {
    // Prepare argv to satisfy the vulnerable program's expected CLI:
    // argv[0] = program name
    // argv[1] = "-depth"
    // argv[2] = bit-depth (use 8 to hit the write at line 257)
    // argv[3] = output TIFF path
    char *argv[] = {
        (char *)"repro",
        (char *)"-depth",
        (char *)"8",
        (char *)"/tmp/out.tif",
        NULL
    };

    // Call the vulnerable main; malloc() inside it returns NULL, leading to
    // a NULL pointer dereference when writing to scan_line.
    return tiff_palette_main(4, argv);
}
