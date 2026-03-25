#include <stdio.h>
#include <stdlib.h>

/* Minimal stand-in types/macros matching the original file's style */
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* This function mirrors the vulnerable argument parsing from
 * contrib/pngminus/png2pnm.c::main. It is intentionally isolated so we don't
 * pull in libpng dependencies. */
static int png2pnm_main(int argc, char *argv[]) {
    FILE *fp_rd = stdin;
    FILE *fp_wr = stdout;
    FILE *fp_al = NULL;
    const char *fname_wr = NULL;
    const char *fname_al = NULL;
    BOOL raw = TRUE;
    BOOL alpha = FALSE;
    int argi;

    for (argi = 1; argi < argc; argi++) {
        if (argv[argi][0] == '-') {
            switch (argv[argi][1]) {
                case 'n':
                    raw = FALSE;
                    break;
                case 'r':
                    raw = TRUE;
                    break;
                case 'a':
                    /* Vulnerable branch: increments argi and immediately
                     * uses argv[argi] without checking it exists. If -a is the
                     * last argument, argv[argi] is NULL and fopen(NULL, ...) is called. */
                    alpha = TRUE;
                    argi++;
                    if ((fp_al = fopen(argv[argi], "wb")) == NULL) {
                        fname_al = argv[argi];
                        fprintf(stderr, "PNG2PNM\n");
                        fprintf(stderr, "Error:  cannot create alpha-channel file %s\n", argv[argi]);
                        exit(1);
                    }
                    break;
                case 'h':
                case '?':
                    /* usage() stubbed out; not needed for this path */
                    exit(0);
                    break;
                default:
                    fprintf(stderr, "PNG2PNM\n");
                    fprintf(stderr, "Error:  unknown option %s\n", argv[argi]);
                    exit(1);
                    break;
            }
        } else if (fp_rd == stdin) {
            if ((fp_rd = fopen(argv[argi], "rb")) == NULL) {
                fprintf(stderr, "PNG2PNM\n");
                fprintf(stderr, "Error:  file %s does not exist\n", argv[argi]);
                exit(1);
            }
        } else if (fp_wr == stdout) {
            fname_wr = argv[argi];
            if ((fp_wr = fopen(argv[argi], "wb")) == NULL) {
                fprintf(stderr, "PNG2PNM\n");
                fprintf(stderr, "Error:  cannot create file %s\n", argv[argi]);
                exit(1);
            }
        }
    }

    /* In the real program, more work would follow. We won't reach here due to crash. */
    if (fp_al) fclose(fp_al);
    if (fp_rd && fp_rd != stdin) fclose(fp_rd);
    if (fp_wr && fp_wr != stdout) fclose(fp_wr);
    (void)fname_wr; (void)fname_al; (void)raw; (void)alpha;
    return 0;
}

int main(void) {
    /* Craft argv so that -a is the last argument, making argv[argi] NULL
     * after argi++ inside the 'a' case. */
    char *argv[] = { (char*)"reproducer", (char*)"-a", NULL };
    int argc = 2; /* argv[0], argv[1] == "-a"; argv[2] == NULL */

    /* This call will enter the vulnerable branch and pass NULL to fopen, which
     * typically triggers a null-pointer dereference inside libc. */
    return png2pnm_main(argc, argv);
}
