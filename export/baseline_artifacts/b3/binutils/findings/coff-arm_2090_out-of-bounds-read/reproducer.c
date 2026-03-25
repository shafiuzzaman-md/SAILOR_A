// Standalone C reproducer for out-of-bounds-read in bfd_arm_process_before_allocation
// Vulnerability: negative symndx (< -1) used to index obj_coff_sym_hashes(abfd)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Minimal type stubs to mimic BFD structures
struct coff_link_hash_entry {
    int symbol_class;
};

struct asection {
    size_t reloc_count;
    struct asection *next;
};

typedef struct bfd {
    struct asection *sections;
    size_t sym_hashes_size; // size of the symbol hash table
    struct coff_link_hash_entry **sym_hashes; // symbol hash table
} bfd;

struct bfd_link_info { int dummy; };

struct internal_reloc {
    long r_symndx;          // The (possibly negative) symbol index
    unsigned short r_type;  // Relocation type
};

// Macros/stubs mirroring the original code expectations
#define BFD_ASSERT(x) do { (void)(x); } while (0)
#define obj_conv_table_size(abfd) ((abfd)->sym_hashes_size)
#define obj_coff_sym_hashes(abfd) ((abfd)->sym_hashes)

// Symbol classes and relocation type constants (minimal)
#define C_THUMBEXTFUNC 1
#define C_EXT 2
#define C_STAT 3
#define C_LABEL 4
#define ARM_26 1
#define ARM_THUMB23 2

// Stub error handler
static void _bfd_error_handler(const char *fmt, bfd *abfd, long symndx) {
    (void)abfd;
    fprintf(stderr, fmt, (void*)abfd, symndx);
    fputc('\n', stderr);
}

// Stub glue recorders
static void record_arm_to_thumb_glue(struct bfd_link_info *info, struct coff_link_hash_entry *h) {
    (void)info; (void)h;
}
static void record_thumb_to_arm_glue(struct bfd_link_info *info, struct coff_link_hash_entry *h) {
    (void)info; (void)h;
}

// Stub that returns relocations. We craft r_symndx = -2 to trigger the bug.
static struct internal_reloc *
bfd_coff_read_internal_relocs(bfd *abfd, struct asection *sec, bool unused1, void *unused2, bool unused3, void *unused4)
{
    (void)abfd; (void)sec; (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    size_t n = sec->reloc_count;
    struct internal_reloc *rels = (struct internal_reloc *)malloc(n * sizeof(*rels));
    if (!rels) {
        perror("malloc rels");
        exit(1);
    }
    // One relocation with a malformed symbol index < -1
    rels[0].r_symndx = -2;  // This bypasses the (symndx == -1) check but is negative
    rels[0].r_type = 0;     // Type doesn't matter for triggering OOB
    return rels;
}

// Vulnerable function (trimmed to the relevant parts)
bool bfd_arm_process_before_allocation(bfd *abfd, struct bfd_link_info *info)
{
    (void)info; // Not needed for trigger
    struct asection *sec = abfd->sections;

    if (sec == NULL)
        return true;

    for (; sec != NULL; sec = sec->next) {
        struct internal_reloc *i;
        struct internal_reloc *rel;

        if (sec->reloc_count == 0)
            continue;

        // Load the relocs
        i = bfd_coff_read_internal_relocs(abfd, sec, true, NULL, false, NULL);
        BFD_ASSERT(i != 0);

        for (rel = i; rel < i + sec->reloc_count; ++rel) {
            unsigned short r_type = rel->r_type;
            long symndx;
            struct coff_link_hash_entry *h;

            symndx = rel->r_symndx;

            // If the relocation is not against a symbol it cannot concern us.
            if (symndx == -1)
                continue;

            // Range check only checks upper bound; negative values pass through.
            if (symndx >= (long)obj_conv_table_size(abfd)) {
                _bfd_error_handler("%pB: illegal symbol index in reloc: %ld", abfd, symndx);
                continue;
            }

            // BUG: Negative symndx indexes before the start of the array
            h = obj_coff_sym_hashes(abfd)[symndx]; // Out-of-bounds read when symndx < 0

            // The rest is irrelevant for the trigger, but keep minimal logic.
            if (h == NULL)
                continue;

            switch (r_type) {
                case ARM_26:
                    if (h->symbol_class == C_THUMBEXTFUNC)
                        record_arm_to_thumb_glue(info, h);
                    break;
                case ARM_THUMB23:
                    switch (h->symbol_class) {
                        case C_EXT:
                        case C_STAT:
                        case C_LABEL:
                            record_thumb_to_arm_glue(info, h);
                            break;
                        default:
                            ;
                    }
                    break;
                default:
                    ;
            }
        }
        // Intentionally leak i for simplicity (mirrors FIXME in original comment)
    }

    return true;
}

int main(void)
{
    // Prepare a minimal bfd with one section and a tiny symbol hash table
    bfd *ab = (bfd *)calloc(1, sizeof(*ab));
    if (!ab) { perror("calloc bfd"); return 1; }

    struct asection *sec = (struct asection *)calloc(1, sizeof(*sec));
    if (!sec) { perror("calloc section"); return 1; }
    sec->reloc_count = 1; // One relocation entry
    sec->next = NULL;

    ab->sections = sec;

    // Allocate a very small symbol hash table to make OOB easy to hit with ASan
    ab->sym_hashes_size = 1;
    ab->sym_hashes = (struct coff_link_hash_entry **)malloc(ab->sym_hashes_size * sizeof(*ab->sym_hashes));
    if (!ab->sym_hashes) { perror("malloc sym_hashes"); return 1; }
    ab->sym_hashes[0] = NULL; // Contents don't matter; access itself should fault under ASan

    struct bfd_link_info info = {0};

    // This call triggers the out-of-bounds read when indexing with symndx = -2
    (void)bfd_arm_process_before_allocation(ab, &info);

    // Cleanup (not reached if ASan aborts on OOB)
    free(ab->sym_hashes);
    free(sec);
    free(ab);

    return 0;
}
