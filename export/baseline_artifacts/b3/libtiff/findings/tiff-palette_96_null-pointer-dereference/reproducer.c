#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

/* Simple SCALE macro to satisfy compilation; it's not reached before the crash */
#define SCALE(x) ((uint16_t)(x))

static void lower_address_space_limit(void) {
    struct rlimit rl;
    rl.rlim_cur = 0;  /* soft limit: 0 bytes of virtual memory available for future allocations */
    rl.rlim_max = 0;  /* hard limit */
    /* Intentionally ignore failure; if it succeeds, mallocs will fail, triggering the bug */
    setrlimit(RLIMIT_AS, &rl);
}

/* Minimal reproduction of the vulnerable logic from contrib/dbs/tiff-palette.c:main */
static void trigger_palette_null_deref(int bits_per_pixel) {
    int nchunks = 0, chunk_size = 0; /* chunk_size is unused but kept for fidelity */
    size_t cmsize;
    uint16_t *red, *green, *blue;
    int i;

    switch (bits_per_pixel)
    {
        case 8:
            nchunks = 16;
            chunk_size = 32;
            break;
        case 4:
            nchunks = 4;
            chunk_size = 128;
            break;
        case 2:
            nchunks = 2;
            chunk_size = 256;
            break;
        case 1:
            nchunks = 2;
            chunk_size = 256;
            break;
        default:
            return; /* mimic Usage() early-exit; not needed for the trigger */
    }

    if (bits_per_pixel != 1)
        cmsize = (size_t)nchunks * (size_t)nchunks;
    else
        cmsize = 2;

    /* Vulnerable allocations: no NULL checks, will be dereferenced below */
    red = (uint16_t *)malloc(cmsize * sizeof(uint16_t));
    green = (uint16_t *)malloc(cmsize * sizeof(uint16_t));
    blue = (uint16_t *)malloc(cmsize * sizeof(uint16_t));

    switch (bits_per_pixel)
    {
        case 8:
            for (i = 0; i < (int)cmsize; i++)
            {
                if (i < 32)
                    red[i] = 0;                 /* NULL deref here when malloc fails */
                else if (i < 64)
                    red[i] = SCALE(36);
                else if (i < 96)
                    red[i] = SCALE(73);
                else if (i < 128)
                    red[i] = SCALE(109);
                else if (i < 160)
                    red[i] = SCALE(146);
                else if (i < 192)
                    red[i] = SCALE(182);
                else if (i < 224)
                    red[i] = SCALE(219);
                else if (i < 256)
                    red[i] = SCALE(255);

                if ((i % 32) < 4)
                    green[i] = 0;
                else if (i < 8)
                    green[i] = SCALE(36);
                else if ((i % 32) < 12)
                    green[i] = SCALE(73);
                else if ((i % 32) < 16)
                    green[i] = SCALE(109);
                else if ((i % 32) < 20)
                    green[i] = SCALE(146);
                else if ((i % 32) < 24)
                    green[i] = SCALE(182);
                else if ((i % 32) < 28)
                    green[i] = SCALE(219);
                else if ((i % 32) < 32)
                    green[i] = SCALE(255);

                if ((i % 4) == 0)
                    blue[i] = SCALE(0);
                else if ((i % 4) == 1)
                    blue[i] = SCALE(85);
                else if ((i % 4) == 2)
                    blue[i] = SCALE(170);
                else if ((i % 4) == 3)
                    blue[i] = SCALE(255);
            }
            break;
        case 4:
        case 2:
        case 1:
        default:
            /* Not needed for the crash; the 8-bpp path already triggers it. */
            break;
    }
}

int main(void) {
    /* Ensure ASan and runtime are initialized before starving memory */
    /* Now starve the process of virtual memory so subsequent malloc() calls fail */
    lower_address_space_limit();

    /* Use 8 bits-per-pixel to follow the exact crashing path at red[i] = 0 */
    trigger_palette_null_deref(8);

    return 0;
}
