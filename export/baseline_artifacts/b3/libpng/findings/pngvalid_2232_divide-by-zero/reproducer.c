// Standalone C reproducer for divide-by-zero in normalize_color_encoding
// Build (as provided):
//   clang -fsanitize=address -g -O0 -I/tmp/libpng_upstream reproducer.c -L/tmp/libpng_upstream/build/.libs -ltiff -lm -o reproducer

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <signal.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

// Weak reference so the program still links/run even if feenableexcept is unavailable
extern int feenableexcept(int) __attribute__((weak));

// Minimal type definitions to mirror contrib/libtests/pngvalid.c
typedef struct {
    double X, Y, Z;
} CIE_color;

typedef struct {
    double gamma;            /* Encoding (file) gamma of space */
    CIE_color red, green, blue; /* End points */
} color_encoding;

// Direct copy of the vulnerable logic (static in pngvalid.c)
static void normalize_color_encoding(color_encoding *encoding)
{
    const double whiteY = encoding->red.Y + encoding->green.Y + encoding->blue.Y;

    if (whiteY != 1)
    {
        // If whiteY == 0.0 this performs a floating-point divide-by-zero
        encoding->red.X   /= whiteY;
        encoding->red.Y   /= whiteY;
        encoding->red.Z   /= whiteY;
        encoding->green.X /= whiteY;
        encoding->green.Y /= whiteY;
        encoding->green.Z /= whiteY;
        encoding->blue.X  /= whiteY;
        encoding->blue.Y  /= whiteY;
        encoding->blue.Z  /= whiteY;
    }
}

static void fpe_handler(int sig, siginfo_t *si, void *ucontext)
{
    (void)si; (void)ucontext;
    const char *msg = "Caught SIGFPE (floating-point divide-by-zero) while normalizing color encoding\n";
    write(2, msg, strlen(msg));
    _Exit(0); // Exit to clearly indicate the divide-by-zero was triggered
}

int main(void)
{
    // Install a SIGFPE handler to clearly surface the FP divide-by-zero if the platform traps it.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = fpe_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGFPE, &sa, NULL);

    // Enable trapping of floating-point divide-by-zero if supported (GNU extension)
    if (feenableexcept) {
        feenableexcept(FE_DIVBYZERO);
    }

    color_encoding enc;
    memset(&enc, 0, sizeof(enc));

    // Craft input so that whiteY == red.Y + green.Y + blue.Y == 0.0, and whiteY != 1.0
    // This causes division by zero in all nine components below when the condition is true.
    enc.red.X = 0.3;   enc.red.Y = 0.0; enc.red.Z = 0.2;
    enc.green.X = 0.4; enc.green.Y = 0.0; enc.green.Z = 0.3;
    enc.blue.X = 0.3;  enc.blue.Y = 0.0; enc.blue.Z = 0.5;

    // Sanity print
    double whiteY = enc.red.Y + enc.green.Y + enc.blue.Y; // == 0.0
    fprintf(stderr, "Before normalize: whiteY=%.17g (expect 0), condition (whiteY != 1)=%s\n",
            whiteY, (whiteY != 1.0) ? "true" : "false");

    // This call triggers the divide-by-zero when whiteY == 0
    normalize_color_encoding(&enc);

    // If FP exceptions are not trapped on this platform, show that components became inf
    if (isinf(enc.red.X) || isinf(enc.red.Y) || isinf(enc.red.Z) ||
        isinf(enc.green.X) || isinf(enc.green.Y) || isinf(enc.green.Z) ||
        isinf(enc.blue.X) || isinf(enc.blue.Y) || isinf(enc.blue.Z)) {
        fprintf(stderr, "Observed infinities after normalization due to division by zero. Bug reproduced.\n");
        return 0;
    }

    // If we reach here without infinities, print values (should not happen with crafted input)
    fprintf(stderr, "Unexpected: no infinities. Values: R(%.3f,%.3f,%.3f) G(%.3f,%.3f,%.3f) B(%.3f,%.3f,%.3f)\n",
            enc.red.X, enc.red.Y, enc.red.Z,
            enc.green.X, enc.green.Y, enc.green.Z,
            enc.blue.X, enc.blue.Y, enc.blue.Z);
    return 0;
}
