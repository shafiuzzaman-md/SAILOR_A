// Standalone reproducer for null-pointer-dereference in opt_provider_param
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

// Minimal OpenSSL-like typedefs/stubs to satisfy the vulnerable function
typedef struct ossl_lib_ctx_st OSSL_LIB_CTX;
typedef struct ossl_provider_st OSSL_PROVIDER;

#define _UC(c) ((unsigned char)(c))

// Stubs for OpenSSL app-layer helpers
static const char *opt_getprog(void) {
    return "repro";
}

// Custom printf that intentionally dereferences %s arguments to expose NULL bugs
static void opt_printf_stderr(const char *fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);

    // Walk the format string and for each %s, touch the pointer to force a crash on NULL
    for (const char *p = fmt; *p != '\0'; ++p) {
        if (*p == '%') {
            ++p;
            if (*p == '%') {
                continue; // literal '%'
            }
            // We only expect %s in this code path
            if (*p == 's') {
                char *s = va_arg(ap, char *);
                // Deliberately touch the pointer to trigger NULL deref if s == NULL
                volatile char ch = s ? s[0] : *((volatile char *)s);
                (void)ch;
            } else {
                // Consume and ignore unknown specifiers conservatively as char*
                (void)va_arg(ap, void *);
            }
        }
    }

    vfprintf(stderr, fmt, ap2);
    va_end(ap2);
    va_end(ap);
}

// OpenSSL memory wrappers
static char *OPENSSL_strdup(const char *s) { return s ? strdup(s) : NULL; }
static void OPENSSL_free(void *p) { free(p); }

// Provider iteration stub: return success but do not find any provider
static int set_prov_param(OSSL_PROVIDER *prov, void *arg) {
    // In real code this would try to set parameters and set p.found = 1 if matched.
    // We never call this to simulate no providers loaded.
    (void)prov; (void)arg; return 1;
}

static OSSL_LIB_CTX g_ctx_obj; // dummy context object
static OSSL_LIB_CTX *app_get0_libctx(void) { return &g_ctx_obj; }

// Simulate provider iteration: returns non-zero (success) but does not call the callback
static int OSSL_PROVIDER_do_all(OSSL_LIB_CTX *ctx,
                                int (*cb)(OSSL_PROVIDER *prov, void *cbarg),
                                void *cbarg) {
    (void)ctx; (void)cb; (void)cbarg;
    // Return success without invoking cb, so no providers are "found"
    return 1;
}

// Vulnerable function extracted/adapted from apps/lib/app_provider.c
static int opt_provider_param(const char *arg)
{
    int ret = 0;
    char *copy = NULL, *tmp;
    struct {
        const char *name;
        char *key;
        char *val;
        int found;
    } p;

    if ((copy = OPENSSL_strdup(arg)) == NULL
        || (p.val = strchr(copy, '=')) == NULL) {
        opt_printf_stderr("%s: malformed '-provparam' option value: '%s'\n",
            opt_getprog(), (char *)arg);
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
        p.name = NULL;
        p.key = copy;
    }

    /* The key must not be empty */
    if (*p.key == '\0') {
        opt_printf_stderr("%s: malformed '-provparam' option value: '%s'\n",
            opt_getprog(), (char *)arg);
        goto end;
    }

    p.found = 0;
    ret = OSSL_PROVIDER_do_all(app_get0_libctx(), set_prov_param, (void *)&p);
    if (ret == 0) {
        opt_printf_stderr("%s: Error setting provider '%s' parameter '%s'\n",
            opt_getprog(), (char *)p.name, (char *)p.key);
    } else if (p.found == 0) {
        // Vulnerable line: p.name can be NULL when no provider specified and none found
        opt_printf_stderr("%s: No provider named '%s' is loaded\n",
            opt_getprog(), (char *)p.name);
        ret = 0;
    }

end:
    OPENSSL_free(copy);
    return ret;
}

int main(void) {
    // Craft input with no provider specified (no ':') so p.name == NULL
    // Also ensure a '=' is present to pass initial parsing
    const char *arg = "key=value";
    // This will reach the vulnerable print with p.name == NULL and trigger a NULL deref
    (void)opt_provider_param(arg);
    return 0;
}
