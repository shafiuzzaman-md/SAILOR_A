#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by the vulnerable code path. */
typedef uint64_t bfd_vma;
typedef int64_t bfd_signed_vma;

typedef struct outsec {
    bfd_vma vma;
} outsec;

typedef struct asection {
    bfd_vma vma;
    bfd_vma output_offset;
    outsec *output_section;
    unsigned char *contents; /* Not used by the buggy line, but present for realism. */
} asection;

typedef struct arelent {
    bfd_vma r_vaddr;
} arelent;

/* Stub for bfd object. */
typedef struct bfd_stub {
    int dummy;
} bfd;

/* Minimal bfd_get_32 that performs a 4-byte read from the provided pointer. */
static uint32_t bfd_get_32(bfd *abfd, const void *p)
{
    (void)abfd; /* unused */
    const unsigned char *q = (const unsigned char *)p;
    /* This will trigger ASan OOB-read if q points past the end of a buffer. */
    return (uint32_t)q[0]
         | ((uint32_t)q[1] << 8)
         | ((uint32_t)q[2] << 16)
         | ((uint32_t)q[3] << 24);
}

/* Minimal bfd_put_32 to mirror the real signature, not used here. */
static void bfd_put_32(bfd *abfd, uint32_t val, void *p)
{
    (void)abfd;
    unsigned char *q = (unsigned char *)p;
    q[0] = (unsigned char)(val & 0xFF);
    q[1] = (unsigned char)((val >> 8) & 0xFF);
    q[2] = (unsigned char)((val >> 16) & 0xFF);
    q[3] = (unsigned char)((val >> 24) & 0xFF);
}

/*
 * A greatly simplified version of the vulnerable code path in
 * bfd/coff-arm.c:coff_arm_relocate_section focusing on the buggy access:
 *   tmp = bfd_get_32(input_bfd, contents + rel->r_vaddr - input_section->vma);
 */
static void coff_arm_relocate_section_minimal(bfd *input_bfd,
                                              asection *input_section,
                                              unsigned char *contents,
                                              arelent *rel)
{
    /* Compute the unchecked offset and perform the 4-byte read. */
    bfd_vma offset = rel->r_vaddr - input_section->vma;
    /* Vulnerable access: no bounds check on offset vs contents buffer size. */
    volatile uint32_t tmp = bfd_get_32(input_bfd, contents + (size_t)offset);

    /* Use tmp to avoid it being optimized away (even though -O0 is used). */
    fprintf(stderr, "Read value: 0x%08x\n", tmp);
}

int main(void)
{
    /* Allocate a small section contents buffer. */
    const size_t buf_size = 8; /* Tiny buffer to make OOB easy. */
    unsigned char *contents = (unsigned char *)malloc(buf_size);
    if (!contents) {
        perror("malloc");
        return 1;
    }
    memset(contents, 0x41, buf_size);

    /* Set up a fake input section with a base vma. */
    asection input_section;
    memset(&input_section, 0, sizeof(input_section));
    input_section.vma = 0; /* Simplify: make offset equal to r_vaddr. */

    /* Craft a relocation whose r_vaddr points near the end of the buffer.
       We pick an offset of 6 so reading 4 bytes straddles the end of the
       8-byte buffer (bytes 6,7 plus 2 bytes OOB). */
    arelent rel;
    rel.r_vaddr = 6; /* Out-of-bounds for a 4-byte read starting at contents+6. */

    bfd dummy_bfd; /* Unused, but matches the signature. */

    /* This call triggers the out-of-bounds read inside bfd_get_32. */
    coff_arm_relocate_section_minimal(&dummy_bfd, &input_section, contents, &rel);

    free(contents);
    return 0;
}
