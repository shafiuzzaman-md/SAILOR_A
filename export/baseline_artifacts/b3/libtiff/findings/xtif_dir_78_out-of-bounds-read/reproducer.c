#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

/*
   Minimal standalone reimplementation of the vulnerable path from
   contrib/tags/xtif_dir.c:_XTIFFPrintDirectory to trigger the
   varargs type mismatch: fprintf("%lu", (uint32_t)).

   We avoid including libtiff headers and instead provide the
   necessary stubs and minimal structures to compile and run.
*/

typedef struct TIFF TIFF; /* opaque placeholder */

/* Example field identifiers (minimal) */
#define FIELD_EXAMPLE_MULTI   0x01
#define FIELD_EXAMPLE_SINGLE  0x02
#define FIELD_EXAMPLE_ASCII   0x04

/* Flag used in the example code; value is irrelevant here */
#define TIFFPRINT_MYMULTIDOUBLES 0x0001

/* Minimal directory and extension state used by the example */
typedef struct {
    double    *xd_example_multi;
    uint16_t   xd_num_multi;
    uint32_t   xd_example_single; /* important: 32-bit type */
    char      *xd_example_ascii;
} XTIFFDirectory;

typedef struct {
    XTIFFDirectory xtif_dir;
} xtiff;

/* Global fake extension state and field-set mask */
static xtiff g_xt;
static unsigned g_field_mask = 0;

/* Stubs/macros from contrib/tags/xtif_dir.c environment */
#define XTIFFDIR(tif) (&g_xt)
#define PARENT(xt, member) (NULL)

static int TIFFFieldSet(TIFF *tif, int field)
{
    (void)tif;
    return (g_field_mask & field) != 0;
}

static void _TIFFprintAsciiTag(FILE *fd, const char *name, const char *value)
{
    if (fd && name && value)
        fprintf(fd, "%s: %s\n", name, value);
}

/*
 * Directly adapted from contrib/tags/xtif_dir.c lines 38-85.
 * The critical bug is at the fprintf using "%lu" for a uint32_t value.
 */
static void _XTIFFPrintDirectory(TIFF *tif, FILE *fd, long flags)
{
    xtiff *xt = XTIFFDIR(tif);
    XTIFFDirectory *xd = &xt->xtif_dir;
    int i, num;

    /* call the inherited method (stubbed out) */
    if (PARENT(xt, printdir))
        ; /* no-op in this reproducer */

    fprintf(fd, "--My Example Tags--\n");

    if (TIFFFieldSet(tif, FIELD_EXAMPLE_MULTI))
    {
        fprintf(fd, "  My Multi-Valued Doubles:");
        if (flags & TIFFPRINT_MYMULTIDOUBLES)
        {
            double *value = xd->xd_example_multi;

            num = xd->xd_num_multi;
            fprintf(fd, "(");
            for (i = 0; i < num; i++)
                fprintf(fd, " %lg", *value++);
            fprintf(fd, ")\n");
        }
        else
            fprintf(fd, "(present)\n");
    }

    if (TIFFFieldSet(tif, FIELD_EXAMPLE_SINGLE))
    {
        /* BUG: xd_example_single is uint32_t but printed with %lu (unsigned long) */
        fprintf(fd, "  My Single Long Tag:  %lu\n", xd->xd_example_single);
    }

    if (TIFFFieldSet(tif, FIELD_EXAMPLE_ASCII))
    {
        _TIFFprintAsciiTag(fd, "My ASCII Tag", xd->xd_example_ascii);
    }
}

int main(void)
{
    /* Craft state so the vulnerable fprintf is reached */
    g_field_mask = FIELD_EXAMPLE_SINGLE; /* Only the SINGLE field is set */

    /* Place a recognizable 32-bit pattern; upper 32 bits (if read) will be garbage */
    g_xt.xtif_dir.xd_example_single = 0x41424344u; /* 'ABCD' */

    /* Use a real FILE* to exercise fprintf. stdout is fine. */
    _XTIFFPrintDirectory(NULL, stdout, 0);

    /* Repeat a few times to increase chance of catching UB under sanitizers */
    for (int i = 0; i < 10; i++) {
        _XTIFFPrintDirectory(NULL, stdout, 0);
    }

    return 0;
}
