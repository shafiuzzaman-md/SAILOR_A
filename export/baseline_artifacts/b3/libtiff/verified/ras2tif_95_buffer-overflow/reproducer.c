// Standalone C reproducer for buffer-overflow in contrib/ras/ras2tif.c:main
// CWE-787: Writing into a string literal via sprintf

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

// Minimal stubs/typedefs mirroring the original file context
typedef int boolean;
#define True 1
#define False 0

typedef unsigned short u_short;
typedef struct TIFF TIFF;      // stub, not used before crash
typedef struct Pixrect Pixrect; // stub, not used before crash
typedef struct { int dummy; } colormap_t; // stub, not used before crash

// Global variables as in the original file (not required to trigger, but included for fidelity)
boolean Verbose = False;
boolean dummyinput = False;
char *pname;

// Error helpers (not used, but declared in original)
void error(char *s1, char *s2) {
    fprintf(stderr, s1, pname, s2);
    exit(1);
}
void usage() { error("usage: %s -[vq] [-|rasterfile] TIFFfile\n", NULL); }

// Reimplementation of the vulnerable code path from ras2tif.c:main
static int ras2tif_main(int argc, char **argv) {
    // Local declarations mirroring the original function (unused before crash)
    char *inf = NULL;
    char *outf = NULL;
    FILE *fp;
    int depth, i;
    long row;
    TIFF *tif;
    Pixrect *pix;        // The Sun Pixrect (stubbed)
    colormap_t Colormap; // The Pixrect Colormap (stubbed)
    u_short red[256], green[256], blue[256];
    struct tm *ct;
    struct timeval tv;
    long width, height;
    long rowsperstrip;
    int year;
    short photometric;
    short samplesperpixel;
    short bitspersample;
    int bpsl;
    static char *version = "ras2tif 1.0";
    // Vulnerable: pointer to string literal (read-only storage)
    static char *datetime = "1990:01:01 12:00:00";

    // This sequence is identical in spirit to the original code and leads to the bug
    gettimeofday(&tv, NULL);
    ct = localtime(&tv.tv_sec);
    year = 1900 + ct->tm_year;

    // BUG: Writing into read-only memory pointed to by 'datetime'
    // This triggers a crash (e.g., SIGSEGV) or ASan error.
    sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d",
            year, ct->tm_mon + 1, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

    // Unreachable if the overflow triggers as expected
    puts(datetime);
    return 0;
}

int main(int argc, char **argv) {
    pname = (argc > 0) ? argv[0] : (char*)"reproducer";
    return ras2tif_main(argc, argv);
}