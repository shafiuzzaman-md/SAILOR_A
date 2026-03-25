#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* Minimal type stubs to mirror the vulnerable function signature. */
typedef struct bfd bfd;
struct bfd { int dummy; };

struct bfd_link_info { int dummy; };

typedef struct asection asection;
struct asection {
  int reloc_count;
};

struct internal_syment {
  uint64_t n_value;
};

struct xcoff_link_hash_entry { int dummy; };

/* Minimal howto struct with only the fields touched around the bug site. */
struct reloc_howto_struct {
  unsigned int bitsize;
  unsigned int size;
  uint64_t src_mask;
  uint64_t dst_mask;
  int complain_on_overflow;
};

/* Dummy relocation type values. Only R_REF is checked before the memcpy. */
#define R_REF 0
#define R_POS 1
#define R_NEG 2

/* Small howto table to demonstrate OOB read when indexed by a large r_type. */
static const struct reloc_howto_struct xcoff64_howto_table[] = {
  { 8,  1, 0xFFull,         0xFFull,         0 },  /* index 0 */
  { 16, 2, 0xFFFFull,       0xFFFFull,       0 },  /* index 1 */
  { 32, 4, 0xFFFFFFFFull,   0xFFFFFFFFull,   0 },  /* index 2 */
  { 64, 8, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull, 0 } /* index 3 */
};

/* Minimal internal relocation struct as used by the vulnerable loop. */
struct internal_reloc {
  uint32_t r_type;
  uint32_t r_size;
  uint64_t r_vaddr;
  int32_t  r_symndx;
};

/* Vulnerable function reimplemented minimally to reproduce the OOB read. */
bool xcoff64_ppc_relocate_section(bfd *output_bfd,
                                  struct bfd_link_info *info,
                                  bfd *input_bfd,
                                  asection *input_section,
                                  unsigned char *contents,
                                  struct internal_reloc *relocs,
                                  struct internal_syment *syms,
                                  asection **sections)
{
  (void)output_bfd; (void)info; (void)input_bfd; (void)contents; (void)syms; (void)sections;

  struct internal_reloc *rel = relocs;
  struct internal_reloc *relend = rel + input_section->reloc_count;

  for (; rel < relend; rel++) {
    /* Skip R_REF just like the original code. */
    if (rel->r_type == R_REF)
      continue;

    /* BUG: No bounds check when indexing xcoff64_howto_table by r_type. */
    struct reloc_howto_struct howto;
    memcpy(&howto, &xcoff64_howto_table[rel->r_type], sizeof(howto));

    /* We don't need to proceed further; the memcpy above is enough to
       trigger an out-of-bounds read under ASan. */
  }
  return true;
}

int main(void)
{
  /* Set up minimal inputs to reach the vulnerable memcpy. */
  bfd out_bfd = {0};
  struct bfd_link_info info = {0};
  bfd in_bfd = {0};

  asection sec = {0};
  sec.reloc_count = 1; /* One relocation to process. */

  unsigned char contents[16] = {0};

  /* Craft a relocation with r_type that is OUT OF BOUNDS for the howto table. */
  struct internal_reloc rels[1];
  rels[0].r_type = 4;   /* xcoff64_howto_table has valid indices 0..3. Index 4 is OOB. */
  rels[0].r_size = 0;
  rels[0].r_vaddr = 0;
  rels[0].r_symndx = -1;  /* Avoid symbol lookup path, not needed here. */

  struct internal_syment syms_stub[1];
  syms_stub[0].n_value = 0;
  asection *sections_stub[1];
  sections_stub[0] = &sec;

  fprintf(stderr, "About to trigger out-of-bounds read via r_type = %u (table size = %zu)\n",
          rels[0].r_type, sizeof(xcoff64_howto_table)/sizeof(xcoff64_howto_table[0]));

  /* Call the vulnerable function. Under ASan, this should report a global-buffer-overflow. */
  (void)xcoff64_ppc_relocate_section(&out_bfd, &info, &in_bfd, &sec,
                                     contents, rels, syms_stub, sections_stub);

  /* If we got here without ASan aborting, print a message. */
  fprintf(stderr, "If you see this without an ASan report, the bug did not reproduce.\n");
  return 0;
}