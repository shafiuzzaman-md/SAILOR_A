#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-in types/macros to mimic the relevant libpng fields/behavior */
typedef struct png_struct_def {
    size_t width;
    size_t height;
    size_t rowbytes;
    size_t info_rowbytes;
    unsigned int pixel_depth; /* bits per pixel */
    unsigned char *prev_row;
} png_struct;

typedef struct png_info_def {
    size_t next_frame_width;
    size_t next_frame_height;
    size_t rowbytes;
    unsigned int pixel_depth; /* bits per pixel */
} png_info;

/* As in libpng: number of bytes needed to hold 'width' pixels at 'bits' bpp */
#define PNG_ROWBYTES(bits, width) (((size_t)(bits) * (size_t)(width) + 7) >> 3)

/* Vulnerable function (reduced to the relevant logic) */
void png_read_reinit(png_struct *png_ptr, png_info *info_ptr)
{
    png_ptr->width = info_ptr->next_frame_width;
    png_ptr->height = info_ptr->next_frame_height;
    png_ptr->rowbytes = PNG_ROWBYTES(png_ptr->pixel_depth, png_ptr->width);
    if (png_ptr->info_rowbytes != 0)
        png_ptr->info_rowbytes = info_ptr->rowbytes =
            PNG_ROWBYTES(info_ptr->pixel_depth, png_ptr->width);
    if (png_ptr->prev_row)
        /* Heap-buffer-overflow when new rowbytes > originally allocated prev_row */
        memset(png_ptr->prev_row, 0, png_ptr->rowbytes + 1);
}

int main(void)
{
    png_struct png;
    png_info info;
    memset(&png, 0, sizeof(png));
    memset(&info, 0, sizeof(info));

    /* Set an initial small frame and allocate prev_row for that small size */
    png.pixel_depth = 32; /* 32 bits/pixel typical RGBA */
    size_t old_width = 1; /* tiny first frame */
    size_t old_rowbytes = PNG_ROWBYTES(png.pixel_depth, old_width); /* 4 bytes */
    png.prev_row = (unsigned char*)malloc(old_rowbytes + 1); /* 5 bytes total */
    if (!png.prev_row) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    memset(png.prev_row, 0xAA, old_rowbytes + 1);

    /* Prepare next frame with a much larger width so rowbytes grows a lot */
    info.next_frame_width = 1024;   /* large enough to exceed previous alloc */
    info.next_frame_height = 1;     /* height doesn't matter for this overflow */
    info.pixel_depth = png.pixel_depth; /* typical propagation */

    /* Trigger: png_read_reinit will recompute rowbytes (4096 for 1024x32bpp)
       and then memset(prev_row, 0, rowbytes+1) which overflows the 5-byte alloc */
    png_read_reinit(&png, &info);

    /* If we got here without ASan aborting, print something (unlikely) */
    printf("Completed without ASan abort (unexpected)\n");

    free(png.prev_row);
    return 0;
}