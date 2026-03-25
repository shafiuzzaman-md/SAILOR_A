#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal re-declarations to model the vulnerable path in libpng's png_free_data */

typedef struct png_struct_def {
    int dummy;
} png_struct, *png_structp;

typedef struct png_unknown_chunk_def {
    unsigned char *data;
} png_unknown_chunk;

typedef struct png_info_def {
    png_unknown_chunk *unknown_chunks;
    int unknown_chunks_num;
    unsigned int free_me;
} png_info, *png_infop;

/* Bit used by png_free_data to decide whether to free unknown chunks */
#define PNG_FREE_UNKN 0x1u

/* Stub that mimics libpng's png_free wrapper */
static void png_free(png_structp png_ptr, void *ptr) {
    (void)png_ptr; /* unused in this stub */
    free(ptr);
}

/* Vulnerable function excerpt modeled after libpng's png_free_data unknown chunks block */
static void png_free_data(png_structp png_ptr, png_infop info_ptr, unsigned int mask, int num)
{
    if (info_ptr->unknown_chunks != NULL && ((mask & PNG_FREE_UNKN) & info_ptr->free_me) != 0)
    {
        if (num != -1)
        {
            /* BUG: no bounds check for num against unknown_chunks_num */
            png_free(png_ptr, info_ptr->unknown_chunks[num].data); /* OOB read of pointer here */
            info_ptr->unknown_chunks[num].data = NULL;             /* OOB write to pointer here */
        }
        else
        {
            int i;
            for (i = 0; i < info_ptr->unknown_chunks_num; i++)
                png_free(png_ptr, info_ptr->unknown_chunks[i].data);

            png_free(png_ptr, info_ptr->unknown_chunks);
            info_ptr->unknown_chunks = NULL;
            info_ptr->unknown_chunks_num = 0;
        }
    }
}

int main(void)
{
    png_struct png;
    png_info info;

    memset(&png, 0, sizeof(png));
    memset(&info, 0, sizeof(info));

    /* Set up a single unknown chunk so unknown_chunks_num == 1 */
    info.unknown_chunks_num = 1;
    info.unknown_chunks = (png_unknown_chunk*)malloc(sizeof(png_unknown_chunk) * info.unknown_chunks_num);
    if (!info.unknown_chunks) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    info.unknown_chunks[0].data = (unsigned char*)malloc(16);
    if (!info.unknown_chunks[0].data) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    memset(info.unknown_chunks[0].data, 0xAA, 16);

    /* Mark that unknown chunks are allowed to be freed */
    info.free_me = PNG_FREE_UNKN;

    /* Choose an out-of-range index to trigger OOB on unknown_chunks[num] */
    int out_of_range_index = 5; /* unknown_chunks_num == 1, so 5 is out of bounds */

    printf("Triggering png_free_data with out-of-range num=%d (unknown_chunks_num=%d)\n",
           out_of_range_index, info.unknown_chunks_num);

    /* This will perform an out-of-bounds access on info.unknown_chunks[num] */
    png_free_data(&png, &info, PNG_FREE_UNKN, out_of_range_index);

    /* Cleanup the valid chunk to avoid leaks if we reached here */
    if (info.unknown_chunks && info.unknown_chunks_num > 0 && info.unknown_chunks[0].data) {
        free(info.unknown_chunks[0].data);
    }
    free(info.unknown_chunks);

    return 0;
}
