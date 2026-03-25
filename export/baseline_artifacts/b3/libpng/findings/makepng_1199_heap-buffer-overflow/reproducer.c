#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal typedefs to mirror libpng types used in the snippet */
typedef uint8_t png_byte;
typedef uint32_t png_uint_32;

/* Stub: check parameter count (from makepng.c) */
static void check_param_count(int nparams, int min)
{
    if (nparams < min) {
        fprintf(stderr, "not enough params\n");
        exit(1);
    }
}

/* Stub: load_file is not used in this reproducer, but referenced by insert_iCCP */
static size_t load_file(const char *path, png_byte **out)
{
    (void)path;
    *out = NULL;
    return 0;
}

/* Stub: emulate makepng.c's fake data loader; here we simply allocate the
 * requested number of bytes from a decimal string.
 */
static size_t load_fake(const char *arg, png_byte **out)
{
    char *end = NULL;
    unsigned long len = strtoul(arg, &end, 10);
    if (end == arg) {
        /* Not a number; for safety just abort. */
        fprintf(stderr, "load_fake: bad length '%s'\n", arg);
        exit(1);
    }
    *out = (png_byte*)malloc(len);
    if (!*out && len != 0) {
        perror("malloc");
        exit(1);
    }
    /* Fill with a pattern to make ASan's reports clearer */
    if (*out && len > 0) memset(*out, 0xAA, len);
    return (size_t)len;
}

/* Emulate libpng's png_save_uint_32: store a 32-bit big-endian integer. */
void png_save_uint_32(png_byte *buf, png_uint_32 i)
{
    /* This will write 4 bytes to buf regardless of buf size. */
    buf[0] = (png_byte)((i >> 24) & 0xff);
    buf[1] = (png_byte)((i >> 16) & 0xff);
    buf[2] = (png_byte)((i >> 8) & 0xff);
    buf[3] = (png_byte)(i & 0xff);
}

/* Optional: emulate libpng's png_get_uint_32 (not used in overflow path) */
static png_uint_32 png_get_uint_32(const png_byte *buf)
{
    return ((png_uint_32)buf[0] << 24) |
           ((png_uint_32)buf[1] << 16) |
           ((png_uint_32)buf[2] << 8)  |
           ((png_uint_32)buf[3]);
}

/* Reconstructed vulnerable function: contrib/libtests/makepng.c: insert_iCCP */
static void insert_iCCP(int nparams, char **params)
{
    png_byte *profile = NULL;
    png_uint_32 proflen = 0;
    int result;

    check_param_count(nparams, 2);

    switch (params[1][0])
    {
       case '<':
          {
             size_t filelen = load_file(params[1]+1, &profile);
             if (filelen > 0xfffffffc) /* Maximum profile length */
             {
                fprintf(stderr, "%s: file too long (%lu) for an ICC profile\n",
                   params[1]+1, (unsigned long)filelen);
                exit(1);
             }

             proflen = (png_uint_32)filelen;
          }
          break;

       case '0': case '1': case '2': case '3': case '4':
       case '5': case '6': case '7': case '8': case '9':
          {
             size_t fake_len = load_fake(params[1], &profile);

             if (fake_len > 0) /* else a simple parameter */
             {
                if (fake_len > 0xffffffff) /* Maximum profile length */
                {
                   fprintf(stderr,
                      "%s: fake data too long (%lu) for an ICC profile\n",
                      params[1], (unsigned long)fake_len);
                   exit(1);
                }
                proflen = (png_uint_32)(fake_len & ~3U);
                /* Always fix up the profile length. */
                /* VULNERABILITY: if fake_len < 4, 'profile' has fewer than 4 bytes. */
                png_save_uint_32(profile, proflen);
                break;
             }
          }

       default:
          fprintf(stderr, "--insert iCCP \"%s\": unrecognized\n", params[1]);
          fprintf(stderr, "  use '<' to read a file: \"<filename\"\n");
          exit(1);
    }

    result = 1;

    if (proflen & 3)
    {
       fprintf(stderr,
          "makepng: --insert iCCP %s: profile length made a multiple of 4\n",
          params[1]);

       while (proflen & 3)
          profile[proflen++] = 0;
    }

    if (profile != NULL && proflen > 3)
    {
       png_uint_32 prof_header = png_get_uint_32(profile);

       if (prof_header != proflen)
       {
          fprintf(stderr, "--insert iCCP %s: profile length field wrong:\n",
             params[1]);
          fprintf(stderr, "  actual %lu, recorded value %lu (corrected)\n",
             (unsigned long)proflen, (unsigned long)prof_header);
          png_save_uint_32(profile, proflen);
       }
    }

    /* Cleanup to avoid leaks in this small reproducer */
    free(profile);
}

int main(void)
{
    /* Set up parameters to hit the digit case with a tiny fake profile. */
    char *params[2];
    params[0] = (char*)"iCCP"; /* unused label */
    params[1] = (char*)"1";    /* fake_len = 1 byte -> overflow when writing 4 bytes */

    /* This call triggers the heap-buffer-overflow in png_save_uint_32 */
    insert_iCCP(2, params);

    /* If the overflow didn't abort, indicate completion */
    fprintf(stderr, "Done (overflow should have been reported by ASan).\n");
    return 0;
}
