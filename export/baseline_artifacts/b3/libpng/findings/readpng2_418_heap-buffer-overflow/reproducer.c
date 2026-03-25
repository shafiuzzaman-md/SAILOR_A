// Standalone reproducer for heap-buffer-overflow in contrib/gregbook/readpng2.c:readpng2_row_callback
// It mimics the truncated row allocation and calls png_progressive_combine_row
// which writes a full row into an undersized buffer.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal libpng-like type aliases
typedef unsigned char png_byte;
typedef uint32_t png_uint_32;

typedef struct png_struct_s png_struct; // forward decl

// Minimal main program info struct as used by readpng2.c
typedef struct mainprog_info_s {
    png_byte **row_pointers;                        // array of row buffers
    int pass;                                       // current pass
    void (*mainprog_init)(void);                    // not used in this reproducer
    void (*mainprog_display_row)(png_uint_32 row);  // called after combining
    void (*mainprog_finish_display)(void);          // not used in this reproducer
} mainprog_info;

// Minimal png_struct to carry the progressive ptr and the full row size
struct png_struct_s {
    void *progressive_ptr;   // user pointer (mainprog_info*)
    size_t full_rowbytes;    // actual full row size libpng would write
};

// Stubs for the libpng API used by readpng2.c
static void *png_get_progressive_ptr(png_struct *png_ptr) {
    return png_ptr->progressive_ptr;
}

// This mimics libpng writing the full row size into the destination row buffer.
// In the real bug, this writes more than was allocated when an earlier int
// truncation caused a smaller allocation.
static void png_progressive_combine_row(png_struct *png_ptr,
                                        png_byte *old_row,
                                        png_byte *new_row) {
    size_t n = png_ptr->full_rowbytes;
    // Deliberately write full_rowbytes bytes to trigger overflow if old_row is smaller
    for (size_t i = 0; i < n; i++) {
        old_row[i] = new_row ? new_row[i] : 0; // behave like copy of full row
    }
}

// Dummy callbacks used by mainprog_info
static void dummy_init(void) { }
static void dummy_display_row(png_uint_32 row) { (void)row; }
static void dummy_finish(void) { }

// The vulnerable callback from contrib/gregbook/readpng2.c (trimmed to essentials)
static void readpng2_row_callback(png_struct *png_ptr, png_byte *new_row,
                                  png_uint_32 row_num, int pass) {
    mainprog_info *mainprog_ptr;

    if (!new_row)
        return;

    // retrieve the pointer to our special-purpose struct so we can access
    // the old rows and image-display callback function
    mainprog_ptr = (mainprog_info *)png_get_progressive_ptr(png_ptr);

    // save the pass number for optional use by the front end
    mainprog_ptr->pass = pass;

    // have libpng either combine the new row data with the existing row data
    // from previous passes (if interlaced) or else just copy the new row
    // into the main program's image buffer
    png_progressive_combine_row(png_ptr, mainprog_ptr->row_pointers[row_num], new_row);

    // finally, call the display routine in the main program with the number
    // of the row we just updated
    (*mainprog_ptr->mainprog_display_row)(row_num);

    return;
}

int main(void) {
    // Simulate an image of height 1 and a very large true row size (e.g., after color/bit depth expansion)
    const png_uint_32 height = 1;
    const size_t full_rowbytes = 1024; // "real" row size libpng will write

    // Simulate the bug: allocate destination row using a truncated int value (too small)
    // In the real code, this happens when size_t rowbytes is stored in an int and truncated.
    int truncated_rowbytes = 16; // much smaller than full_rowbytes

    // Allocate the main program structure and its row pointers
    mainprog_info *mp = (mainprog_info *)calloc(1, sizeof(*mp));
    if (!mp) {
        fprintf(stderr, "alloc mainprog_info failed\n");
        return 1;
    }

    mp->row_pointers = (png_byte **)calloc(height, sizeof(png_byte *));
    if (!mp->row_pointers) {
        fprintf(stderr, "alloc row_pointers failed\n");
        return 1;
    }

    // Undersized allocation due to truncation
    mp->row_pointers[0] = (png_byte *)malloc((size_t)truncated_rowbytes);
    if (!mp->row_pointers[0]) {
        fprintf(stderr, "alloc row 0 failed\n");
        return 1;
    }

    // Prepare a new_row buffer with the full amount of data libpng would provide
    png_byte *new_row = (png_byte *)malloc(full_rowbytes);
    if (!new_row) {
        fprintf(stderr, "alloc new_row failed\n");
        return 1;
    }
    memset(new_row, 0x41, full_rowbytes);

    // Set up dummy callbacks
    mp->mainprog_init = dummy_init;
    mp->mainprog_display_row = dummy_display_row;
    mp->mainprog_finish_display = dummy_finish;

    // Create a minimal png_struct carrying the progressive pointer and the true row size
    png_struct png;
    png.progressive_ptr = (void *)mp;
    png.full_rowbytes = full_rowbytes;

    // Trigger the vulnerable path: combine/copy full row into undersized buffer
    // This should cause an ASan heap-buffer-overflow in png_progressive_combine_row
    readpng2_row_callback(&png, new_row, 0 /*row_num*/, 0 /*pass*/);

    // Cleanup (likely not reached if ASan aborts on overflow)
    free(new_row);
    free(mp->row_pointers[0]);
    free(mp->row_pointers);
    free(mp);

    return 0;
}
