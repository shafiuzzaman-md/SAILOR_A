#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal type re-declarations to mirror the relevant parts of bfd/coff-alpha.c */
typedef uint64_t bfd_vma;
typedef size_t bfd_size_type;

typedef struct bfd_section asection;

struct bfd_section {
    void *used_by_bfd;              /* storage for section-private data */
    bfd_vma output_offset;
    struct bfd_section *output_section; /* points to output section; we just reuse same type */
    bfd_vma vma;                    /* used when this section acts as output_section */
    bfd_size_type size;
    const char *name;
};

struct ecoff_section_tdata {
    bfd_vma gp; /* The field that gets dereferenced without NULL check */
};

struct ecoff_tdata {
    struct bfd_section **symndx_to_section;
    void *sym_hashes;
    int issued_multiple_gp_warning;
};

struct bfd {
    struct ecoff_tdata ecoff;
};

struct bfd_link_info;
struct bfd_link_callbacks {
    void (*warning)(struct bfd_link_info *info,
                    const char *msg,
                    const char *arg,
                    struct bfd *abfd,
                    void *sect,
                    int reloc);
};

struct bfd_link_info {
    int relocatable; /* 0 => not relocatable, triggers the vulnerable path */
    struct bfd_link_callbacks *callbacks;
    struct bfd *output_bfd;
};

/* Index for the .lita section in the symndx_to_section array. */
enum { RELOC_SECTION_LITA = 7 };

/* Stubs mimicking the BFD API used by alpha_relocate_section. */
static struct ecoff_tdata *ecoff_data(struct bfd *abfd) {
    return &abfd->ecoff;
}

static struct ecoff_section_tdata *ecoff_section_data(struct bfd *abfd, struct bfd_section *sec) {
    (void)abfd;
    return (struct ecoff_section_tdata *)sec->used_by_bfd;
}

static void *bfd_zalloc(struct bfd *abfd, size_t size) {
    (void)abfd; (void)size;
    /* Simulate out-of-memory to trigger the bug: return NULL. */
    return NULL;
}

static bfd_vma _bfd_get_gp_value(struct bfd *abfd) {
    (void)abfd;
    return 0; /* any value is fine; 0 simplifies path */
}

static void _bfd_set_gp_value(struct bfd *abfd, bfd_vma gp) {
    (void)abfd; (void)gp;
}

static int bfd_link_relocatable(struct bfd_link_info *info) {
    return info->relocatable;
}

/* Optional warning callback stub. */
static void warn_stub(struct bfd_link_info *info, const char *msg, const char *arg,
                      struct bfd *abfd, void *sect, int reloc) {
    (void)info; (void)msg; (void)arg; (void)abfd; (void)sect; (void)reloc;
}

/* A reduced version of alpha_relocate_section focusing on the vulnerable logic. */
static void alpha_relocate_section(struct bfd *output_bfd,
                                   struct bfd_link_info *info,
                                   struct bfd *input_bfd) {
    struct bfd_section **symndx_to_section = ecoff_data(input_bfd)->symndx_to_section;

    struct bfd_section *lita_sec = symndx_to_section[RELOC_SECTION_LITA];
    bfd_vma gp = _bfd_get_gp_value(output_bfd);

    if (!bfd_link_relocatable(info) && lita_sec != NULL) {
        struct ecoff_section_tdata *lita_sec_data;

        /* Attempt to fetch per-section data; none present initially. */
        lita_sec_data = ecoff_section_data(input_bfd, lita_sec);
        if (lita_sec_data == NULL) {
            /* Simulated bfd_zalloc failure leaves lita_sec_data NULL,
               but code assigns it to used_by_bfd anyway. */
            lita_sec_data = (struct ecoff_section_tdata *)bfd_zalloc(input_bfd, sizeof(*lita_sec_data));
            lita_sec->used_by_bfd = lita_sec_data;
        }

        /* Vulnerable NULL dereference: lita_sec_data is NULL if bfd_zalloc failed. */
        if (lita_sec_data->gp != 0) {
            gp = lita_sec_data->gp;
        } else {
            /* Not reached; deref above already crashes under ASan. */
            ;
        }
        _bfd_set_gp_value(output_bfd, gp);
    }
}

int main(void) {
    /* Set up input and output BFDs */
    struct bfd *input_bfd = (struct bfd *)calloc(1, sizeof(struct bfd));
    struct bfd *output_bfd = (struct bfd *)calloc(1, sizeof(struct bfd));
    if (!input_bfd || !output_bfd) {
        fprintf(stderr, "Allocation failure in test harness\n");
        return 1;
    }

    /* Prepare .lita section and place it into the symndx_to_section array. */
    struct bfd_section *lita_out = (struct bfd_section *)calloc(1, sizeof(struct bfd_section));
    struct bfd_section *lita_in = (struct bfd_section *)calloc(1, sizeof(struct bfd_section));
    if (!lita_out || !lita_in) {
        fprintf(stderr, "Allocation failure for sections\n");
        return 1;
    }

    lita_in->name = ".lita";
    lita_in->size = 16;
    lita_in->output_offset = 0;
    lita_in->output_section = lita_out;

    lita_out->vma = 0x10000; /* any value is fine */

    /* Create a symndx_to_section array and point the LITA index to our section. */
    size_t arr_sz = RELOC_SECTION_LITA + 1;
    struct bfd_section **symndx_to_section = (struct bfd_section **)calloc(arr_sz, sizeof(*symndx_to_section));
    if (!symndx_to_section) {
        fprintf(stderr, "Allocation failure for symndx array\n");
        return 1;
    }
    symndx_to_section[RELOC_SECTION_LITA] = lita_in;

    ecoff_data(input_bfd)->symndx_to_section = symndx_to_section;

    /* Prepare link info indicating non-relocatable link (to enter vulnerable block). */
    static struct bfd_link_callbacks cbs = { warn_stub };
    struct bfd_link_info info;
    memset(&info, 0, sizeof(info));
    info.relocatable = 0; /* non-relocatable => !bfd_link_relocatable(info) is true */
    info.callbacks = &cbs;
    info.output_bfd = output_bfd;

    /* Trigger the vulnerable code path: bfd_zalloc returns NULL and code dereferences it. */
    alpha_relocate_section(output_bfd, &info, input_bfd);

    /* If we somehow didn't crash, indicate failure. */
    fprintf(stderr, "Did not trigger NULL dereference as expected.\n");
    return 0;
}
