// Standalone C reproducer for OOB read in MY(swap_ext_reloc_in)
// Compile with: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Minimal stub types to mimic BFD structures used by the vulnerable function.
typedef size_t bfd_size_type;

typedef struct howto {
  int val;
} howto;

// arelent with a howto pointer as in BFD
typedef struct arelent {
  long address;
  howto *howto;
} arelent;

typedef struct asymbol {
  int dummy;
} asymbol;

// aout data scaffolding used (but not actually needed) by the vulnerable code
struct aoutdata { int dummy; };
struct aout_data_struct { struct aoutdata a; };
struct tdata_union { struct aout_data_struct *aout_data; };

typedef struct bfd {
  struct tdata_union tdata;
} bfd;

// External reloc record layout (only fields accessed in the snippet)
struct reloc_ext_external {
  unsigned char r_address[4];
  unsigned char r_index[3];
  unsigned char r_type[1];
  unsigned char r_addend[4];
};

// Global howto table with only 3 valid entries (indices 0..2)
static howto howto_table_ext[3] = {
  { 0x11111111 },
  { 0x22222222 },
  { 0x33333333 }
};

// Bit masks/shifts to decode r_type/r_extern. We keep this simple for the repro.
#define RELOC_EXT_BITS_EXTERN_LITTLE 0x80
#define RELOC_EXT_BITS_TYPE_LITTLE   0x03
#define RELOC_EXT_BITS_TYPE_SH_LITTLE 0

// Helpers/macros used by the function body
static inline int32_t get_sword_le(const unsigned char *p) {
  return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                   ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

#define GET_SWORD(abfd, field) get_sword_le((const unsigned char*)(field))
#define MOVE_ADDRESS(V) do { (void)(V); /* stub: original mutates via macros */ } while (0)

// Error stubs
static void _bfd_error_handler(const char *fmt, const void *abfd, unsigned int x) {
  (void)abfd;
  fprintf(stderr, "_bfd_error_handler: ");
  fprintf(stderr, fmt, abfd, x);
  fprintf(stderr, "\n");
}

enum { bfd_error_wrong_format = 1 };
static void bfd_set_error(int err) {
  fprintf(stderr, "bfd_set_error: %d\n", err);
}

// Vulnerable function modeled after MY(swap_ext_reloc_in)
static void cris_swap_ext_reloc_in(
    bfd *abfd,
    struct reloc_ext_external *bytes,
    arelent *cache_ptr,
    asymbol **symbols,
    bfd_size_type symcount)
{
  (void)symbols; (void)symcount; // Unused in this minimal repro

  unsigned int r_index;
  int r_extern;
  unsigned int r_type;
  struct aoutdata *su = &(abfd->tdata.aout_data->a);
  (void)su; // Unused but kept to mirror original code shape

  cache_ptr->address = (GET_SWORD (abfd, bytes->r_address));

  // Build r_index from 3 bytes (little-endian as per original)
  r_index = (((unsigned int) bytes->r_index[2] << 16)
           | ((unsigned int) bytes->r_index[1] << 8)
           |  bytes->r_index[0]);

  r_extern = (0 != (bytes->r_type[0] & RELOC_EXT_BITS_EXTERN_LITTLE));

  r_type = ((bytes->r_type[0] & RELOC_EXT_BITS_TYPE_LITTLE)
          >> RELOC_EXT_BITS_TYPE_SH_LITTLE);

  if (r_type > 2)
    {
      _bfd_error_handler("%pB: unsupported relocation type imported: %#x", abfd, r_type);
      bfd_set_error(bfd_error_wrong_format);
    }

  // BUG: No clamp or early return; this can point past the end when r_type==3
  cache_ptr->howto = howto_table_ext + r_type;

  if (r_extern && r_index > symcount)
    {
      _bfd_error_handler("%pB: bad relocation record imported: %d", abfd, r_index);
      bfd_set_error(bfd_error_wrong_format);
      r_extern = 0;
      r_index = 0; // N_ABS in real code
    }

  MOVE_ADDRESS (GET_SWORD (abfd, bytes->r_addend));
}

int main(void) {
  // Allocate minimal bfd and aout scaffolding
  bfd *abfd = (bfd*)calloc(1, sizeof(bfd));
  if (!abfd) return 1;
  abfd->tdata.aout_data = (struct aout_data_struct*)calloc(1, sizeof(struct aout_data_struct));
  if (!abfd->tdata.aout_data) return 1;

  // Prepare a relocation record with r_type == 3 (out of valid range 0..2)
  struct reloc_ext_external rec;
  memset(&rec, 0, sizeof(rec));
  rec.r_type[0] = 0x03; // extern bit 0, type bits == 3 => triggers the bug

  // Prepare cache entry and symbols
  arelent rel;
  memset(&rel, 0, sizeof(rel));
  asymbol *syms = NULL;

  // Call the vulnerable function
  cris_swap_ext_reloc_in(abfd, &rec, &rel, &syms, 10);

  // Force dereference of the out-of-bounds howto pointer to trigger ASan report
  // When r_type==3, rel.howto points one past the 3-element howto_table_ext.
  // This read should be in a redzone and flagged as global-buffer-overflow.
  printf("Triggered value: %x\n", rel.howto->val);

  free(abfd->tdata.aout_data);
  free(abfd);
  return 0;
}
