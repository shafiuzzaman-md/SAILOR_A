// Standalone reproducer for the null-pointer-dereference in
// bfd/coff-loongarch64.c: in_reloc_p (howto == NULL)

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Minimal stand-ins for BFD types used by the vulnerable code.
typedef struct bfd bfd; // Opaque in real BFD, unused here.

typedef struct reloc_howto_type {
    bool pc_relative;
} reloc_howto_type;

// This mirrors the empty howto table in coff-loongarch64.c
static reloc_howto_type pe_loongarch64_std_reloc_howto[] = {
    // Intentionally empty: no relocation howtos are defined.
};

// Cache struct holding a howto pointer as used by RTYPE2HOWTO
typedef struct {
    reloc_howto_type *howto;
} arelent_cache;

// This macro is exactly what the backend defines: it always sets howto to NULL.
#define RTYPE2HOWTO(cache_ptr, dst) ((cache_ptr)->howto = NULL)

// Vulnerable function copied to reproduce the crash: unconditionally
dereferences howto->pc_relative.
static bool in_reloc_p(bfd *abfd /* ATTRIBUTE_UNUSED */, reloc_howto_type *howto) {
    (void)abfd; // Unused
    // Vulnerability: howto may be NULL; dereferencing it crashes.
    return !howto->pc_relative;
}

// Simulate processing a relocation in the LoongArch64 PE/COFF backend.
// Encountering any relocation uses RTYPE2HOWTO, which sets howto = NULL,
// and then in_reloc_p is called with that NULL howto, triggering the crash.
static void simulate_loongarch64_pe_coff_reloc(void) {
    arelent_cache cache;
    memset(&cache, 0, sizeof(cache));

    // In the real backend, the relocation type would be translated to a howto
    // via RTYPE2HOWTO. Because the howto table is empty, RTYPE2HOWTO sets it to NULL.
    RTYPE2HOWTO(&cache, 0 /* dummy relocation type */);

    // This call will dereference cache.howto (which is NULL) and crash.
    // AddressSanitizer will report a null-pointer-dereference here.
    (void)in_reloc_p(NULL, cache.howto);
}

int main(void) {
    // Show that the howto table is indeed empty, like in the backend.
    if (sizeof(pe_loongarch64_std_reloc_howto) != 0) {
        fprintf(stderr, "Unexpected non-empty howto table in reproducer.\n");
        return 1;
    }

    // Trigger the vulnerable path.
    simulate_loongarch64_pe_coff_reloc();

    // We should never reach here; if we do, the environment didn't trigger the crash.
    printf("If you see this, the crash did not occur as expected.\n");
    return 0;
}
