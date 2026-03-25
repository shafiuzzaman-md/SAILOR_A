#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal forward declaration so we don't need to include tiff headers */
typedef struct TIFF TIFF;

/* This function replicates the vulnerable option parsing from
 * contrib/addtiffo/addtiffo.c:main(), up through the buggy loop. */
static int addtiffo_like_main(int argc, char **argv)
{
    int anOverviews[100];
    int nOverviewCount = 0;
    int bUseSubIFD = 0;
    TIFF *hTIFF; /* Unused here, but mirrors original code */
    const char *pszResampling = "nearest";

    if (argc < 2)
    {
        printf("Usage: addtiffo [-r {nearest,average,mode}]\n"
               "                tiff_filename [resolution_reductions]\n");
        return 1;
    }

    /* Vulnerable loop from line ~95 in the original file. */
    while (argv[1][0] == '-')
    {
        if (strcmp(argv[1], "-subifd") == 0)
        {
            bUseSubIFD = 1;
            argv++;
            argc--;
        }
        else if (strcmp(argv[1], "-r") == 0)
        {
            /* This advances past the option and its value without checking
             * that a value actually exists. With only "-r" present and no
             * following value, this moves argv to point to the terminating
             * NULL, and reduces argc to 0. */
            argv += 2;
            argc -= 2;
            pszResampling = *argv; /* becomes NULL in our crafted case */
        }
        else
        {
            fprintf(stderr, "Incorrect parameters\n");
            return 1;
        }
    }

    /* Not reached in our reproducer because the loop condition will access
     * out-of-bounds memory and ASan will report it. */
    (void)anOverviews; (void)nOverviewCount; (void)bUseSubIFD; (void)hTIFF; (void)pszResampling;
    return 0;
}

int main(void)
{
    /* Craft argv as if the program was invoked with exactly two arguments:
     * argv[0] = program name, argv[1] = "-r", and no value following -r.
     * This matches the vulnerable scenario described. */
    int argc = 2;
    char **argv = (char**)malloc(sizeof(char*) * 3);
    if (!argv) return 1;
    argv[0] = (char*)"repro";
    argv[1] = (char*)"-r";
    argv[2] = NULL; /* like a real argv terminator */

    /* Call the function that reproduces the original main's parsing logic. */
    int ret = addtiffo_like_main(argc, argv);

    /* We likely won't reach here because the out-of-bounds read occurs when
     * the while condition re-evaluates argv[1][0] after argv has been
     * advanced past the end of the array. */
    free(argv);
    return ret;
}
