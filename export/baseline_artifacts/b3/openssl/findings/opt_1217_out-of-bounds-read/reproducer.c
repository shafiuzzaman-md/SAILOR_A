#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Self-contained reproducer for the opt_help OOB-read bug. */

/* Sentinel control entries as non-string pointer constants (invalid for strlen). */
#define OPT_HELP_STR    ((const char *)0x1)
#define OPT_SECTION_STR ((const char *)0x2)
#define OPT_PARAM_STR   ((const char *)0x3)
#define OPT_MORE_STR    ((const char *)0x4)

#define MAX_OPT_HELP_WIDTH 80

typedef struct options_st {
    const char *name;     /* Option name or sentinel */
    int         valtype;  /* Value type marker, compared to '-' in code */
    const char *help;     /* Help text */
} OPTIONS;

static const char *prog = "repro";

static int opt_printf_stderr(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(stderr, fmt, ap);
    va_end(ap);
    return r;
}

static const char *valtype2param(const OPTIONS *o)
{
    (void)o;
    return "param";
}

/* Minimal stub for opt_print; content doesn't matter for triggering the bug. */
static void opt_print(const OPTIONS *o, int doingparams, int width)
{
    (void)o;
    (void)doingparams;
    (void)width;
}

void opt_help(const OPTIONS *list)
{
    const OPTIONS *o;
    int i, sawparams = 0, width = 5;
    int standard_prolog;

    /* Starts with its own help message? */
    standard_prolog = list[0].name != OPT_HELP_STR;

    /* Find the widest help. */
    for (o = list; o->name; o++) {
        if (o->name == OPT_MORE_STR)
            continue;

        /* BUG: No exclusion for OPT_HELP_STR, OPT_SECTION_STR, OPT_PARAM_STR here. */
        i = 2 + (int)strlen(o->name);  /* This strlen on sentinel pointer causes OOB-read */
        if (o->valtype != '-')
            i += 1 + (int)strlen(valtype2param(o));

        if (i > width)
            width = i;
    }

    if (width > MAX_OPT_HELP_WIDTH)
        width = MAX_OPT_HELP_WIDTH;

    if (standard_prolog) {
        opt_printf_stderr("Usage: %s [options]\n", prog);
        if (list[0].name != OPT_SECTION_STR)
            opt_printf_stderr("Valid options are:\n");
    }

    /* Now let's print. */
    for (o = list; o->name; o++) {
        if (o->name == OPT_PARAM_STR)
            sawparams = 1;
        opt_print(o, sawparams, width);
    }
}

int main(void)
{
    /* The first entry is a sentinel (OPT_HELP_STR), which is NOT a C string. */
    static const OPTIONS opts[] = {
        { OPT_HELP_STR, '-', "(help sentinel)" },
        /* Additional sentinels would also trigger the issue if reached: */
        { OPT_SECTION_STR, '-', "(section sentinel)" },
        { OPT_PARAM_STR,   '-', "(param sentinel)" },
        /* Terminate list with NULL name as per iteration condition. */
        { NULL, 0, NULL }
    };

    /* This call will hit strlen(o->name) on a non-string pointer and crash under ASan. */
    opt_help(opts);
    return 0;
}
