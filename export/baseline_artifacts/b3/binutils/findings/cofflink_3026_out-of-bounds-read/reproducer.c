// Standalone C reproducer for OOB read in coff_relocate_section (bfd/cofflink.c:3026)
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer
// The harness mirrors the vulnerable indexing: sec = sections[symndx];

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Minimal stand-ins for BFD-related types used by coff_relocate_section.
typedef struct bfd_section
{
  struct bfd_section *output_section;
  uintptr_t vma;
  size_t output_offset;
} asection;

// Simulated COFF internal symbol entry (only fields we need)
typedef struct internal_syment
{
  int n_scnum;
  long n_value;
} internal_syment;

// Simulated reloc howto type
struct reloc_howto_type
{
  int pc_relative;
  int pcrel_offset;
};

// Simulated link info
struct bfd_link_info
{
  int dummy;
};

// Global absolute section pointer placeholder
static asection abs_sec_storage;
asection *bfd_abs_section_ptr = &abs_sec_storage;

// Stubs for helper functions/macros used in the vulnerable routine
static inline int bfd_link_relocatable(struct bfd_link_info *info) { (void)info; return 0; }
static inline int bfd_is_abs_section(asection *sec) { return sec == bfd_abs_section_ptr; }
static inline int obj_pe(void *bfd) { (void)bfd; return 0; }

static struct reloc_howto_type *
bfd_coff_rtype_to_howto(void *input_bfd,
                        asection *input_section,
                        void *rel,
                        void *h,
                        internal_syment *sym,
                        long *addend)
{
  (void)input_bfd; (void)input_section; (void)rel; (void)h; (void)sym; (void)addend;
  static struct reloc_howto_type howto = { 0, 0 }; // Ensure we go through the path we want
  return &howto;
}

// A minimal reproduction of the vulnerable logic in coff_relocate_section.
// Parameters are simplified for the purposes of triggering the OOB read.
bool coff_relocate_section(void *input_bfd,
                           asection *input_section,
                           void *rel,
                           void *h,                      // Force NULL to follow the vulnerable path
                           internal_syment *syms,        // Symbol table base
                           asection **sections,          // Array of section pointers
                           long symndx,                  // Crafted index from relocation
                           struct bfd_link_info *info)   // Link info
{
  (void)input_bfd;
  (void)input_section;
  (void)rel;

  internal_syment *sym = syms + symndx; // Assume caller made this safe

  long addend;
  if (sym != NULL && sym->n_scnum != 0)
    addend = - sym->n_value;
  else
    addend = 0;

  struct reloc_howto_type *howto = bfd_coff_rtype_to_howto(input_bfd, input_section, rel, h, sym, &addend);
  if (howto == NULL)
    return false;

  if (howto->pc_relative && howto->pcrel_offset)
  {
    if (bfd_link_relocatable(info))
      return true;
    if (sym != NULL && sym->n_scnum != 0)
      addend += sym->n_value;
  }

  long val = 0;
  asection *sec = NULL;

  if (h == NULL)
  {
    if (symndx == -1)
    {
      sec = bfd_abs_section_ptr;
      val = 0;
    }
    else
    {
      // Vulnerable access: no bounds check on symndx vs number of sections
      // This read triggers ASAN when symndx is out of range of the 'sections' array.
      sec = sections[symndx];

      // The rest mimics the original flow (not necessary for the crash)
      if (sec == NULL || bfd_is_abs_section(sec))
        return true;

      val = (long)(sec->output_section->vma + sec->output_offset + (uintptr_t)sym->n_value);
      if (!obj_pe(input_bfd))
        val -= (long)sec->vma;
    }
  }
  else
  {
    // Not needed for this reproducer.
  }

  // Prevent optimizing the variables away
  volatile long sink = val + addend + (long)(uintptr_t)sec;
  (void)sink;
  return true;
}

int main(void)
{
  // Prepare a small sections array
  const size_t sections_count = 4; // Intentionally small
  asection **sections = (asection **)calloc(sections_count, sizeof(*sections));
  if (!sections) {
    perror("calloc sections");
    return 1;
  }

  // Populate valid in-bounds entries to avoid NULL short-circuiting before the bug
  for (size_t i = 0; i < sections_count; i++) {
    sections[i] = (asection *)calloc(1, sizeof(asection));
    if (!sections[i]) {
      perror("calloc section elem");
      return 1;
    }
    sections[i]->output_section = sections[i];
    sections[i]->vma = 0x1000 * (i + 1);
    sections[i]->output_offset = 0x10 * i;
  }

  // Choose a symndx that is well beyond the bounds of 'sections'
  const long symndx = 1000; // Out-of-range index

  // Allocate a sufficiently large symbol table so that accesses via 'symndx' are in-bounds here
  size_t syms_count = (size_t)symndx + 1;
  internal_syment *syms = (internal_syment *)calloc(syms_count, sizeof(*syms));
  if (!syms) {
    perror("calloc syms");
    return 1;
  }
  // Make the selected symbol look defined and with a value, mimicking a normal symbol
  syms[symndx].n_scnum = 1;
  syms[symndx].n_value = 0x1234;

  struct bfd_link_info info = {0};

  // Trigger the vulnerable path: h == NULL, symndx != -1, and symndx out of range of 'sections'.
  // This will perform: sec = sections[symndx]; causing an out-of-bounds read.
  (void)coff_relocate_section(NULL, NULL, NULL, NULL, syms, sections, symndx, &info);

  // Cleanup (won't be reached if ASAN aborts as expected)
  for (size_t i = 0; i < sections_count; i++)
    free(sections[i]);
  free(sections);
  free(syms);

  return 0;
}
