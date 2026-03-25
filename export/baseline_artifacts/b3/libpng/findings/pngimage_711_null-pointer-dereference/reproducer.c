#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Minimal reconstruction of the pieces from contrib/libtests/pngimage.c
 * to reach the vulnerable fprintf with a NULL %s argument.
 */

typedef enum error_level
{
    INFORMATION = 0,
    LIBPNG_WARNING,
    APP_WARNING,
    APP_FAIL,
    LIBPNG_ERROR,
    LIBPNG_BUG,
    APP_ERROR,
    USER_ERROR,
    INTERNAL_ERROR,
    VERBOSE,
    WARNINGS,
    ERRORS,
    QUIET
} error_level;

/* In the real code this masks verbosity in options; any nonzero level will do. */
#define LEVEL_MASK 0xF

struct display {
    unsigned int results;
    unsigned int options;
    const char *filename;
    const char *operation; /* Intentionally left NULL to trigger the bug */
    int transforms;
};

/* Stubs to satisfy references inside display_log; they won't be called
 * because we force transforms == 0.
 */
static int is_combo(int tr) { (void)tr; return 0; }
static const char* transform_name(int tr) { (void)tr; return "stub"; }

/* Vulnerable function reconstructed from pngimage.c:display_log */
static void display_log(struct display *dp, error_level level, const char *fmt, ...)
{
    (void)fmt; /* Unused in this minimal reproducer */

    dp->results |= 1U << level;

    if (level > (error_level)(dp->options & LEVEL_MASK))
    {
        const char *lp;
        switch (level)
        {
            case INFORMATION:    lp = "information"; break;
            case LIBPNG_WARNING: lp = "warning(libpng)"; break;
            case APP_WARNING:    lp = "warning(pngimage)"; break;
            case APP_FAIL:       lp = "error(continuable)"; break;
            case LIBPNG_ERROR:   lp = "error(libpng)"; break;
            case LIBPNG_BUG:     lp = "bug(libpng)"; break;
            case APP_ERROR:      lp = "error(pngimage)"; break;
            case USER_ERROR:     lp = "error(user)"; break;
            case INTERNAL_ERROR: /* fallthrough */
            case VERBOSE: case WARNINGS: case ERRORS: case QUIET:
            default:             lp = "bug(pngimage)"; break;
        }

        /* Vulnerable line: dp->operation may be NULL here. */
        fprintf(stderr, "%s: %s: %s",
                dp->filename != NULL ? dp->filename : "<stdin>",
                lp,
                dp->operation);

        /* Keep structure similar; this block won't execute because transforms==0. */
        if (dp->transforms != 0)
        {
            int tr = dp->transforms;
            if (is_combo(tr))
            {
                /* Omitted: combo printing; not needed for reproducer */
                (void)tr;
            }
            else
            {
                fprintf(stderr, "(%s)", transform_name(tr));
            }
        }
        fprintf(stderr, "\n");
    }
}

int main(void)
{
    /* Simulate a freshly initialized display where operation has not yet
     * been set (i.e., NULL). This mirrors display_init behavior.
     */
    struct display d;
    memset(&d, 0, sizeof(d));
    d.filename = NULL;      /* Triggers use of "<stdin>" in fprintf */
    d.operation = NULL;     /* This is the crasher when used with %s */
    d.transforms = 0;       /* Avoids calling transform helpers */
    d.options = 0;          /* Ensures level > (options & LEVEL_MASK) */

    /* Call with a level high enough to enter the logging path. */
    display_log(&d, APP_ERROR, "trigger-before-operation-set");

    return 0;
}
