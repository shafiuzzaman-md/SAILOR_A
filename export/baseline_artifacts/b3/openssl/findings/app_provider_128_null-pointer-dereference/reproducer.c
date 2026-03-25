#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* Minimal re-declarations and stubs to make this self-contained */
#define _UC(c) ((unsigned char)(c))

static const char *opt_getprog(void) {
    return "reproducer";
}

/* Stub OPENSSL strdup/free wrappers */
static char *OPENSSL_strdup(const char *s) { return s ? strdup(s) : NULL; }
static void OPENSSL_free(void *p) { free(p); }

/* Stub app_get0_libctx; real OpenSSL returns an OSSL_LIB_CTX* */
static void *app_get0_libctx(void) { return NULL; }

/* Stub for the callback symbol; not actually used here */
static int set_prov_param(void *prov, void *cbdata) { (void)prov; (void)cbdata; return 1; }

/* Stub OSSL_PROVIDER_do_all that forces an error (returns 0) */
static int OSSL_PROVIDER_do_all(void *libctx,
                               int (*cb)(void *, void *),
                               void *arg) {
    (void)libctx; (void)cb; (void)arg;
    /* Force failure to hit the error printing path */
    return 0;
}

/* Vulnerable printf wrapper: we will intentionally dereference the 2nd %s arg */
static int opt_printf_stderr(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    /* We know the vulnerable call uses: "%s: Error setting provider '%s' parameter '%s'\n" */
    const char *prog = va_arg(ap, const char *);   /* %s -> program name */
    const char *provname = va_arg(ap, const char *);/* %s -> provider name (can be NULL) */
    /* Trigger the null-pointer-dereference explicitly */
    volatile char ch = provname[0]; /* Will crash if provname == NULL */
    (void)ch;
    /* We won't actually print; the crash happens above */
    va_end(ap);
    (void)fmt; (void)prog; /* silence unused warnings if optimized */
    return 0;
}

/* Structure used by opt_provider_param */
struct prov_param_st {
    char *name;  /* provider name, optional; NULL means all */
    char *key;   /* parameter key (may have been split from provider:key) */
    char *val;   /* parameter value (after '=') */
    int found;   /* whether a provider matched */
};

/* Vulnerable function extracted/simplified from apps/lib/app_provider.c */
static int opt_provider_param(const char *arg)
{
    struct prov_param_st p;
    char *copy, *tmp;
    int ret = 0;

    if ((copy = OPENSSL_strdup(arg)) == NULL
        || (p.val = strchr(copy, '=')) == NULL) {
        opt_printf_stderr("%s: malformed '-provparam' option value: '%s'\n",
            opt_getprog(), arg);
        goto end;
    }

    /* Drop whitespace on both sides of the '=' sign */
    *(tmp = p.val++) = '\0';
    while (tmp > copy && isspace(_UC(*--tmp)))
        *tmp = '\0';
    while (isspace(_UC(*p.val)))
        ++p.val;

    /*
     * Split the key on ':', to get the optional provider, empty or missing
     * means all.
     */
    if ((p.key = strchr(copy, ':')) != NULL) {
        *p.key++ = '\0';
        p.name = *copy != '\0' ? copy : NULL;
    } else {
        p.name = NULL;   /* no provider specified -> NULL */
        p.key = copy;
    }

    /* The key must not be empty */
    if (*p.key == '\0') {
        opt_printf_stderr("%s: malformed '-provparam' option value: '%s'\n",
            opt_getprog(), arg);
        goto end;
    }

    p.found = 0;
    ret = OSSL_PROVIDER_do_all(app_get0_libctx(), set_prov_param, (void *)&p);
    if (ret == 0) {
        /* Vulnerable: p.name can be NULL, but printed with %s */
        opt_printf_stderr("%s: Error setting provider '%s' parameter '%s'\n",
            opt_getprog(), p.name, p.key);
    } else if (p.found == 0) {
        opt_printf_stderr("%s: No provider named '%s' is loaded\n",
            opt_getprog(), p.name);
        ret = 0;
    }

end:
    OPENSSL_free(copy);
    return ret;
}

int main(void) {
    /*
     * Craft input with no provider before ':' and with '=' present.
     * Example: "param=value". This sets p.name = NULL and p.key = "param".
     * Our OSSL_PROVIDER_do_all stub returns 0 to force the error path that
     * prints p.name with %s, which our opt_printf_stderr then dereferences.
     */
    const char *arg = "param=value"; /* No ':' -> provider omitted -> p.name == NULL */
    /* This call should trigger a NULL dereference inside opt_printf_stderr */
    (void)opt_provider_param(arg);
    return 0;
}
