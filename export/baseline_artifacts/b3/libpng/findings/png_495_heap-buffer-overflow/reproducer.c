#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-in types/defines to mirror libpng structures/flags */
typedef uint32_t png_uint_32;

typedef struct png_struct_def {
    int dummy;
} png_struct;

typedef struct png_text_def {
    char *key; /* The field accessed without bounds checking */
} png_text;

typedef struct png_info_def {
    png_uint_32 free_me;
    png_text *text;
    int num_text;
    int max_text;
} png_info;

#define PNG_TEXT_SUPPORTED 1
#define PNG_FREE_TEXT 0x01

#define png_debug(level, message) ((void)0)

static void png_error(const png_struct *png_ptr, const char *msg) {
    (void)png_ptr;
    fprintf(stderr, "png_error: %s\n", msg);
    abort();
}

/* Stand-in for libpng's png_free: just calls free on the pointer */
static void png_free(const png_struct *png_ptr, void *ptr) {
    (void)png_ptr;
    free(ptr);
}

/* Vulnerable function (reduced to the PNG_TEXT_SUPPORTED block) */
void png_free_data(const png_struct *png_ptr, png_info *info_ptr, png_uint_32 mask, int num)
{
    png_debug(1, "in png_free_data");

    if (png_ptr == NULL || info_ptr == NULL)
        return;

#ifdef PNG_TEXT_SUPPORTED
    /* Free text item num or (if num == -1) all text items */
    if (info_ptr->text != NULL &&
        ((mask & PNG_FREE_TEXT) & info_ptr->free_me) != 0)
    {
        if (num != -1)
        {
            /* BUG: no bounds check on num; may be out of range or negative */
            png_free(png_ptr, info_ptr->text[num].key);
            info_ptr->text[num].key = NULL;
        }
        else
        {
            int i;
            for (i = 0; i < info_ptr->num_text; i++)
                png_free(png_ptr, info_ptr->text[i].key);

            png_free(png_ptr, info_ptr->text);
            info_ptr->text = NULL;
            info_ptr->num_text = 0;
            info_ptr->max_text = 0;
        }
    }
#endif
}

int main(void) {
    /* Set up minimal structures to reach the vulnerable path */
    png_struct *png_ptr = (png_struct *)malloc(sizeof(png_struct));
    png_info *info = (png_info *)malloc(sizeof(png_info));
    if (!png_ptr || !info) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    memset(info, 0, sizeof(*info));

    /* Make PNG_FREE_TEXT active in both mask and info->free_me */
    info->free_me = PNG_FREE_TEXT;

    /* Allocate exactly 1 text entry so index 1 is out-of-bounds */
    info->num_text = 1;
    info->max_text = 1;
    info->text = (png_text *)malloc(sizeof(png_text) * 1);
    if (!info->text) {
        fprintf(stderr, "alloc text failed\n");
        return 1;
    }
    info->text[0].key = (char *)malloc(4);
    if (!info->text[0].key) {
        fprintf(stderr, "alloc key failed\n");
        return 1;
    }
    memcpy(info->text[0].key, "abc", 4); /* small dummy string */

    /* Trigger: pass num = 1 (out-of-bounds; only index 0 is valid) */
    png_free_data(png_ptr, info, PNG_FREE_TEXT, 1);

    /* If the program reaches here without ASan aborting, clean up */
    if (info->text) {
        if (info->text[0].key) free(info->text[0].key);
        free(info->text);
    }
    free(info);
    free(png_ptr);
    return 0;
}
