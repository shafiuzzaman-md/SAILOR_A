// Standalone reproducer for the null-pointer-dereference in apps/gendsa.c:gendsa_main
// It mimics the control flow: load_keyparams() returns NULL for an invalid params file,
// bio_open_owner() succeeds, then EVP_PKEY_get_bits(pkey) dereferences NULL.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// --- Minimal stub types to satisfy references ---
typedef struct BIO { int dummy; } BIO;
typedef struct EVP_PKEY { int bits; } EVP_PKEY;
typedef struct EVP_PKEY_CTX { int dummy; } EVP_PKEY_CTX;
typedef struct EVP_CIPHER { int dummy; } EVP_CIPHER;
typedef struct ENGINE { int dummy; } ENGINE;

// --- Minimal constants used by the original code ---
#define FORMAT_UNDEF 0
#define FORMAT_PEM   1

// --- Global used by BIO_printf/ERR_print_errors in apps ---
static BIO *bio_err = NULL;

// --- Stubbed helper functions mimicking the OpenSSL app layer ---
static int app_RAND_load(void) { return 1; }
static int opt_cipher(const char *name, EVP_CIPHER **enc) { (void)name; (void)enc; return 1; }
static int app_passwd(char *in, char *passoutarg, char *in2, char **passout) {
    (void)in; (void)passoutarg; (void)in2; if (passout) *passout = NULL; return 1;
}

// load_keyparams stub: return NULL to simulate invalid/missing DSA params file
static EVP_PKEY *load_keyparams(const char *file, int format, int loadpub, const char *keytype, const char *desc) {
    (void)format; (void)loadpub; (void)keytype; (void)desc;
    // Simulate failure if file is missing or contents are invalid.
    FILE *f = fopen(file, "rb");
    if (f) {
        // Read a little, but still fail to mimic "invalid parameters".
        char buf[16];
        (void)fread(buf, 1, sizeof(buf), f);
        fclose(f);
    }
    return NULL; // Force failure so pkey becomes NULL
}

// bio_open_owner stub: always succeed by returning a non-NULL BIO
static BIO *bio_open_owner(const char *filename, int format, int private_flag) {
    (void)filename; (void)format; (void)private_flag;
    BIO *b = (BIO *)calloc(1, sizeof(BIO));
    return b; // Non-NULL -> success
}

// Minimal I/O and cleanup stubs
static int BIO_printf(BIO *b, const char *fmt, ...) {
    (void)b; (void)fmt; // Silence unused warnings
    return 0;
}
static void ERR_print_errors(BIO *b) { (void)b; }
static void BIO_free(BIO *b) { free(b); }
static void BIO_free_all(BIO *b) { free(b); }
static void EVP_PKEY_free(EVP_PKEY *p) { free(p); }
static void EVP_PKEY_CTX_free(EVP_PKEY_CTX *ctx) { free(ctx); }
static void EVP_CIPHER_free(EVP_CIPHER *enc) { free(enc); }
static void release_engine(ENGINE *e) { (void)e; }

// Vulnerable accessor: this intentionally dereferences pkey and will crash if pkey==NULL
static int EVP_PKEY_get_bits(EVP_PKEY *pkey) {
    return pkey->bits; // NULL dereference when pkey is NULL
}

// Simplified reproduction of apps/gendsa.c:gendsa_main control flow
int gendsa_main(int argc, char **argv) {
    const char *dsaparams = NULL;
    const char *outfile = "out.pem"; // ensure bio_open_owner() can succeed
    EVP_CIPHER *enc = NULL;
    char *passout = NULL;
    int private_flag = 1;
    BIO *in = NULL, *out = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    int ret = 1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <invalid-or-missing-dsaparam-file>\n", argv[0]);
        return 2;
    }
    dsaparams = argv[1];

    if (!app_RAND_load())
        goto end;
    if (!opt_cipher(NULL, &enc))
        goto end;
    if (!app_passwd(NULL, NULL, NULL, &passout)) {
        BIO_printf(bio_err, "Error getting password\n");
        goto end;
    }

    // This returns NULL to simulate invalid params
    pkey = load_keyparams(dsaparams, FORMAT_UNDEF, 1, "DSA", "DSA parameters");

    // bio_open_owner succeeds (returns non-NULL)
    out = bio_open_owner(outfile, FORMAT_PEM, private_flag);
    if (out == NULL)
        goto end2;

    // Vulnerable NULL dereference (line 138 in the original code)
    int nbits = EVP_PKEY_get_bits(pkey);
    (void)nbits; // never reached under ASan, but silences warnings

    ret = 0;
end:
    if (ret != 0)
        ERR_print_errors(bio_err);
end2:
    BIO_free(in);
    BIO_free_all(out);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    EVP_CIPHER_free(enc);
    release_engine(NULL);
    free(passout);
    return ret;
}

int main(void) {
    // Create a temporary file with invalid contents to simulate a bad DSA params file.
    char tmpfname[] = "/tmp/invalid_dsaparams_XXXXXX";
    int fd = mkstemp(tmpfname);
    if (fd != -1) {
        const char *junk = "not DSA params";
        (void)write(fd, junk, (unsigned)strlen(junk));
        close(fd);
    } else {
        // Fallback to a sure-to-be-missing path
        strcpy(tmpfname, "/nonexistent/invalid_dsaparams");
    }

    char *argv[] = { (char*)"repro", tmpfname, NULL };
    // This will crash with AddressSanitizer due to NULL dereference at EVP_PKEY_get_bits
    return gendsa_main(2, argv);
}
