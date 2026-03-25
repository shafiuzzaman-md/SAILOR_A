#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal logging levels to match the snippet */
enum { LOG_EMERG = 0, LOG_ALERT = 1, LOG_CRIT = 2, LOG_ERR = 3, LOG_WARNING = 4, LOG_NOTICE = 5, LOG_INFO = 6, LOG_DEBUG = 7, LOG_TRACE = 8 };

/* Minimal BIO infrastructure stubs */
typedef struct BIO_st {
    FILE *fp;
    char prefix[80];
    int is_prefix;
    struct BIO_st *next;
} BIO;

static BIO *bio_err = NULL;

static void init_bio_err(void) {
    bio_err = (BIO *)calloc(1, sizeof(BIO));
    if (!bio_err) {
        fprintf(stderr, "alloc failed\n");
        exit(1);
    }
    bio_err->fp = stderr;
    bio_err->is_prefix = 0;
}

void *BIO_f_prefix(void) { return (void *)0x1; }
BIO *BIO_new(void *method) {
    (void)method;
    BIO *b = (BIO *)calloc(1, sizeof(BIO));
    if (b) {
        b->fp = stderr;
        b->is_prefix = 1;
    }
    return b;
}
int BIO_set_prefix(BIO *pre, const char *pfx) {
    if (!pre || !pfx) return 0;
    strncpy(pre->prefix, pfx, sizeof(pre->prefix) - 1);
    pre->prefix[sizeof(pre->prefix) - 1] = '\0';
    return 1;
}
BIO *BIO_push(BIO *top, BIO *below) {
    if (!top) return below;
    top->next = below;
    return top;
}
int BIO_flush(BIO *bio) {
    (void)bio;
    fflush(stderr);
    return 1;
}
BIO *BIO_pop(BIO *bio) {
    if (!bio) return NULL;
    BIO *n = bio->next;
    bio->next = NULL;
    return n;
}
void BIO_free(BIO *b) { free(b); }

int BIO_vprintf(BIO *bio, const char *fmt, va_list ap) {
    char buf[1024];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (bio && bio->is_prefix) fputs(bio->prefix, stderr);
    fputs(buf, stderr);
    return n;
}
int BIO_printf(BIO *bio, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = BIO_vprintf(bio, fmt, ap);
    va_end(ap);
    return n;
}

/* Stub that intentionally crashes when given a NULL for %s, reflecting the bug */
int BIO_snprintf(char *buf, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = 0;
    if (strstr(fmt, "%s") != NULL) {
        const char *s = va_arg(ap, const char *);
        if (s == NULL) {
            /* Trigger the null-pointer-dereference as the vulnerable code would */
            volatile char crash = ((const char *)s)[0];
            (void)crash;
        }
        ret = snprintf(buf, n, fmt, s);
    } else {
        ret = vsnprintf(buf, n, fmt, ap);
    }
    va_end(ap);
    return ret;
}

/* Trace stubs */
int OSSL_trace_enabled(int category) { (void)category; return 0; }
BIO *OSSL_trace_begin(int category) { (void)category; return bio_err; }
void OSSL_trace_end(int category, BIO *out) { (void)category; (void)out; }

/* Implementation from the snippet */
static int verbosity = LOG_INFO;

static void log_with_prefix(const char *prog, const char *fmt, va_list ap) {
    char prefix[80];
    BIO *bio, *pre = BIO_new(BIO_f_prefix());
    (void)BIO_snprintf(prefix, sizeof(prefix), "%s: ", prog); /* prog == NULL triggers crash */
    (void)BIO_set_prefix(pre, prefix);
    bio = BIO_push(pre, bio_err);
    (void)BIO_vprintf(bio, fmt, ap);
    (void)BIO_printf(bio, "\n");
    (void)BIO_flush(bio);
    (void)BIO_pop(pre);
    BIO_free(pre);
}

void trace_log_message(int category, const char *prog, int level, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    if (category >= 0 && OSSL_trace_enabled(category)) {
        BIO *out = OSSL_trace_begin(category);
        (void)BIO_vprintf(out, fmt, ap);
        OSSL_trace_end(category, out);
    } else if (verbosity >= level) {
        log_with_prefix(prog, fmt, ap);
    }

    va_end(ap);
}

int log_set_verbosity(const char *prog, int level) {
    if (level < LOG_EMERG || level > LOG_TRACE) {
        trace_log_message(-1, prog, LOG_ERR, "Invalid verbosity level %d", level);
        return 0;
    }
    verbosity = level;
    return 1;
}

int main(void) {
    init_bio_err();

    /* Trigger path: invalid level causes trace_log_message with prog == NULL */
    (void)log_set_verbosity(NULL, 9999);

    /* If it didn't crash (it should), exit nonzero to indicate failure */
    return 0;
}
