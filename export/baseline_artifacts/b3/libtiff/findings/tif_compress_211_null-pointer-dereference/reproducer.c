#include <stdio.h>
#include <tiffio.h>

/*
 * Minimal TIFFInitMethod stub. It won't be reached because the crash
 * happens before any use of this function, at strlen(name).
 */
static int DummyInit(TIFF *tif, int scheme)
{
    (void)tif;
    (void)scheme;
    return 1;
}

int main(void)
{
    /* Intentionally pass a NULL name to trigger the vulnerability */
    const char *name = NULL;

    /*
     * This call will dereference the NULL pointer via strlen(name)
     * inside TIFFRegisterCODEC (tif_compress.c:211).
     */
    const TIFFCodec *codec = TIFFRegisterCODEC(65000 /* arbitrary scheme */, name, DummyInit);

    /* If the vulnerability is fixed, codec would be NULL or non-NULL depending on behavior. */
    (void)codec;

    /* If we got here without crashing, print a message. */
    printf("TIFFRegisterCODEC returned without crashing (vulnerability likely fixed)\n");
    return 0;
}
