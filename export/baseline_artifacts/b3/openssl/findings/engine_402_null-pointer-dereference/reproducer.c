#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Minimal stub of OpenSSL BIO */
typedef struct bio_st {
    int dummy;
} BIO;

/* Stub: dup_bio_out(FORMAT_TEXT) can fail and return NULL */
#define FORMAT_TEXT 1
static BIO *dup_bio_out(int format) {
    (void)format;
    /* Simulate allocation/open failure: return NULL */
    return NULL;
}

/* Stub of BIO_printf that dereferences the BIO pointer like OpenSSL would */
int BIO_printf(BIO *b, const char *fmt, ...) {
    /* Force a null-pointer dereference when b == NULL, mimicking OpenSSL's BIO use */
    int crash = b->dummy; /* This will segfault if b is NULL */
    (void)crash;

    va_list ap;
    va_start(ap, fmt);
    va_end(ap);
    return 0;
}

/* Minimal ENGINE stubs */
typedef struct engine_st {
    const char *id;
    const char *name;
} ENGINE;

static ENGINE global_engine;

ENGINE *ENGINE_by_id(const char *id) {
    global_engine.id = id;
    global_engine.name = "Stub Engine";
    return &global_engine;
}

const char *ENGINE_get_name(ENGINE *e) { return e ? e->name : ""; }
const char *ENGINE_get_id(ENGINE *e)   { return e ? e->id   : ""; }

/* Vulnerable function modeled after apps/engine.c: engine_main */
int engine_main(int argc, char **argv) {
    (void)argc;
    /* In the real code, 'out' is obtained via dup_bio_out(FORMAT_TEXT) without checking */
    BIO *out = dup_bio_out(FORMAT_TEXT); /* returns NULL -> bug precondition */

    /* Simulate having parsed one engine id from argv or defaults */
    const char *id = (argv && argv[1]) ? argv[1] : "test";

    ENGINE *e = ENGINE_by_id(id);
    const char *name = ENGINE_get_name(e);

    /* Line analogous to engine.c:402 -> dereferences NULL 'out' */
    BIO_printf(out, "(%s) %s\n", id, name);

    return 0;
}

int main(int argc, char **argv) {
    /* Directly invoke the vulnerable path */
    return engine_main(argc, argv);
}
