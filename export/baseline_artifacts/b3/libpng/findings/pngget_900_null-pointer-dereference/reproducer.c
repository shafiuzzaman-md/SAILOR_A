#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal typedefs to mimic libpng types */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

typedef struct png_struct_def png_struct;
typedef struct png_info_def png_info;

struct png_struct_def { int dummy; };

/* Only the fields needed by png_get_eXIf_1 */
struct png_info_def {
    png_uint_32 valid;
    png_uint_32 num_exif;
    png_byte *exif;
};

/* Fake debug macro used by libpng */
#define png_debug1(level, message, arg1) ((void)0)

/* Bit for eXIf validity; any non-zero unique bit works for this reproducer */
#define PNG_INFO_eXIf 0x00000001u

/* This macro ensures our stub is compiled, matching the upstream conditional */
#define PNG_eXIf_SUPPORTED 1

#ifdef PNG_eXIf_SUPPORTED
/* Vulnerable function copied in spirit from pngget.c: it writes to *num_exif
 * without checking if num_exif is NULL when exif != NULL and PNG_INFO_eXIf set.
 */
png_uint_32 png_get_eXIf_1(const png_struct *png_ptr, const png_info *info_ptr,
    png_uint_32 *num_exif, png_byte **exif)
{
    png_debug1(1, "in %s retrieval function", "eXIf");

    if (png_ptr != NULL && info_ptr != NULL &&
        (info_ptr->valid & PNG_INFO_eXIf) != 0 && exif != NULL)
    {
        /* BUG: no check that num_exif != NULL before dereference */
        *num_exif = info_ptr->num_exif;  /* NULL deref when num_exif == NULL */
        *exif = info_ptr->exif;
        return PNG_INFO_eXIf;
    }

    return 0;
}
#endif

int main(void)
{
    /* Set up structures with eXIf info present */
    png_struct png_s;                /* Non-NULL png_ptr */
    png_info info;                   /* Non-NULL info_ptr */
    memset(&png_s, 0, sizeof(png_s));
    memset(&info, 0, sizeof(info));

    /* Populate eXIf data and mark it valid */
    png_byte *exif_data = (png_byte*)malloc(8);
    if (!exif_data) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    for (int i = 0; i < 8; ++i) exif_data[i] = (png_byte)i;

    info.valid |= PNG_INFO_eXIf;     /* Pretend eXIf chunk is present */
    info.num_exif = 8;               /* Number of eXIf bytes */
    info.exif = exif_data;           /* Pointer to eXIf data */

    /* Prepare arguments to trigger the bug */
    png_uint_32 *num_exif_ptr = NULL;  /* Intentionally NULL to trigger NPD */
    png_byte *out_exif = NULL;         /* Non-NULL pointer argument (address below is non-NULL) */

    /* This call will enter the 'if' branch and then dereference num_exif_ptr (NULL) */
    (void)png_get_eXIf_1(&png_s, &info, num_exif_ptr, &out_exif);

    /* Not reached: the above line should crash due to NULL dereference */
    free(exif_data);
    return 0;
}
