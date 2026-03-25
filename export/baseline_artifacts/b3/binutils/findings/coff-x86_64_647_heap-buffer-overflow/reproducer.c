#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by the vulnerable function */
typedef unsigned long long bfd_vma;
typedef unsigned char bfd_byte;
typedef int bfd_boolean;

struct asection;

typedef struct bfd {
    struct asection *sections; /* head of output section list */
} bfd;

typedef struct asection {
    bfd_vma vma;                     /* section VMA (base address) */
    struct asection *output_section; /* mapped output section */
    struct asection *next;           /* next in output list */
    unsigned int index;              /* index for hashing (unused here) */
} asection;

struct internal_reloc {
    bfd_vma r_vaddr; /* relocation address within input section */
    unsigned short r_type; /* unused in this reproducer */
};

/* Stubs to mirror checks in the original code path */
static int bfd_is_abs_section(const asection *sec) { (void)sec; return 0; }
static int discarded_section(const asection *sec) { (void)sec; return 0; }

/* Little-endian 16-bit store, like bfd_putl16 */
static void bfd_putl16(unsigned short val, bfd_byte *p)
{
    p[0] = (unsigned char)(val & 0xff);
    p[1] = (unsigned char)((val >> 8) & 0xff);
}

/* Minimal replica of the vulnerable function logic around the bug site. */
static bfd_boolean coff_pe_amd64_relocate_section(
    bfd *output_bfd,
    void *info,                 /* unused */
    bfd *input_bfd,             /* unused */
    asection *input_section,
    bfd_byte *contents,
    struct internal_reloc *relocs,
    void *syms,                 /* unused */
    asection **sections)
{
    (void)info; (void)input_bfd; (void)syms;

    /* Choose a section to match the earlier code path (sec = sections[symndx]). */
    asection *sec = sections[0];
    if (!sec) return 0;
    if (bfd_is_abs_section(sec)) return 0;
    if (discarded_section(sec)) return 0;

    /* Find index of sec->output_section in output_bfd->sections list. */
    int i = 0, idx = 0;
    asection *s = output_bfd->sections;
    while (s) {
        if (s == sec->output_section) {
            idx = i;
            break;
        }
        i++;
        s = s->next;
    }

    /* Vulnerable write: no bounds check of r_vaddr against input_section/contents. */
    bfd_putl16((unsigned short)idx, contents + (size_t)(relocs[0].r_vaddr - input_section->vma));
    return 1;
}

int main(void)
{
    /* Build an output section list: [out1] -> [out2] */
    asection out1; memset(&out1, 0, sizeof(out1));
    asection out2; memset(&out2, 0, sizeof(out2));
    out1.index = 0; out1.next = &out2;
    out2.index = 1; out2.next = NULL;

    bfd outbfd; memset(&outbfd, 0, sizeof(outbfd));
    outbfd.sections = &out1;

    /* Create an arbitrary section that maps to out2. */
    asection mapped_sec; memset(&mapped_sec, 0, sizeof(mapped_sec));
    mapped_sec.output_section = &out2;

    asection *sections_arr[1] = { &mapped_sec };

    /* Input section with a chosen VMA. */
    asection in_sec; memset(&in_sec, 0, sizeof(in_sec));
    in_sec.vma = 0x1000ULL;

    /* Allocate a small contents buffer (16 bytes). */
    size_t buf_sz = 16;
    bfd_byte *contents = (bfd_byte *)malloc(buf_sz);
    if (!contents) {
        perror("malloc");
        return 1;
    }
    memset(contents, 0x41, buf_sz);

    /* Craft a relocation whose r_vaddr lies outside the input section's contents.
       Set r_vaddr so that (r_vaddr - vma) = buf_sz + 2 -> 2 bytes past the end.
       The vulnerable code will write 2 bytes at that location, triggering ASan. */
    struct internal_reloc rel; memset(&rel, 0, sizeof(rel));
    rel.r_vaddr = in_sec.vma + buf_sz + 2; /* OOB by 2 bytes for a 16-bit store */

    /* Call the vulnerable function. */
    coff_pe_amd64_relocate_section(&outbfd, NULL, NULL, &in_sec, contents, &rel, NULL, sections_arr);

    /* Prevent optimizer from eliding the write; also keep program alive briefly. */
    printf("First byte: %u\n", (unsigned)contents[0]);

    free(contents);
    return 0;
}
