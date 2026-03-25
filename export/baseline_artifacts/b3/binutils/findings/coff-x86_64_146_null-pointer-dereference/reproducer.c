// Standalone C reproducer for NULL pointer dereference in coff_amd64_reloc
// Triggers: *error_message = ... when error_message == NULL under ELF flavour

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef _
#define _(x) x
#endif

// Minimal BFD-like stubs

typedef struct bfd bfd;

typedef enum {
  bfd_target_unknown_flavour = 0,
  bfd_target_coff_flavour = 1,
  bfd_target_elf_flavour = 2
} bfd_flavour;

struct bfd {
  bfd_flavour flavour;
};

static inline bfd_flavour bfd_get_flavour(bfd *abfd) {
  return abfd ? abfd->flavour : bfd_target_unknown_flavour;
}

// Section structure (only fields we need)
struct asection {
  struct asection *output_section; // points to output section
  bfd *owner;                      // owning bfd of the output section
};

typedef struct asection asection;

// Relocation howto and entry stubs

typedef struct reloc_howto_type {
  int type;
  bool pc_relative;
} reloc_howto_type;

typedef struct arelent {
  reloc_howto_type *howto;
  long addend;
  long address;
} arelent;

// Linker hash and info stubs
struct bfd_link_hash_table { int dummy; };

typedef enum {
  bfd_link_hash_undefined = 0,
  bfd_link_hash_defined = 1,
  bfd_link_hash_defweak = 2
} bfd_link_hash_type;

struct bfd_link_hash_entry {
  bfd_link_hash_type type;
  union {
    struct {
      unsigned long long value;
      asection *section;
    } def;
  } u;
};

struct bfd_link_info {
  struct bfd_link_hash_table *hash;
};

static struct bfd_link_info global_link_info;

static struct bfd_link_info* _bfd_get_link_info(bfd *abfd) {
  (void)abfd;
  // Return a non-NULL link_info to reach bfd_link_hash_lookup
  return &global_link_info;
}

static struct bfd_link_hash_entry* bfd_link_hash_lookup(struct bfd_link_hash_table *table,
                                                        const char *name,
                                                        bool create,
                                                        bool copy,
                                                        bool weak)
{
  (void)table; (void)name; (void)create; (void)copy; (void)weak;
  // Simulate "__ImageBase" not found -> return NULL
  return NULL;
}

// Relocation status

typedef enum {
  bfd_reloc_ok = 0,
  bfd_reloc_dangerous = 1,
} bfd_reloc_status_type;

// Minimal constant values we need
#define R_AMD64_IMAGEBASE 0x7fffffff  // Arbitrary distinct value for this test

// Vulnerable function re-implementation (focused on the vulnerable path)
bfd_reloc_status_type coff_amd64_reloc(bfd *abfd,
                                       arelent *reloc_entry,
                                       void *symbol /* unused in this repro */,
                                       void *data   /* unused in this repro */,
                                       asection *input_section,
                                       bfd *output_bfd,
                                       char **error_message)
{
  (void)abfd; (void)symbol; (void)data;

  if (reloc_entry->howto->type == R_AMD64_IMAGEBASE && output_bfd == NULL) {
    bfd *obfd = input_section->output_section->owner;
    switch (bfd_get_flavour(obfd)) {
      case bfd_target_elf_flavour: {
        // Subtract __ImageBase => need it to be defined; simulate it's missing
        struct bfd_link_info *link_info;
        struct bfd_link_hash_entry *h = NULL;
        link_info = _bfd_get_link_info(obfd);
        if (link_info != NULL)
          h = bfd_link_hash_lookup(link_info->hash, "__ImageBase", false, false, true);
        if (h == NULL || (h->type != bfd_link_hash_defined && h->type != bfd_link_hash_defweak)) {
          // Vulnerable write through potentially NULL error_message
          *error_message = (char *) _("R_AMD64_IMAGEBASE with __ImageBase undefined");
          return bfd_reloc_dangerous;
        }
        break;
      }
      default:
        break;
    }
  }

  return bfd_reloc_ok;
}

int main(void) {
  // Prepare an ELF-flavoured output BFD through the input section's output_section->owner
  bfd elf_bfd = { .flavour = bfd_target_elf_flavour };

  asection out_sec = { .output_section = NULL, .owner = &elf_bfd };
  asection in_sec = { .output_section = &out_sec, .owner = NULL };

  // Prepare relocation entry with type R_AMD64_IMAGEBASE
  reloc_howto_type howto = { .type = R_AMD64_IMAGEBASE, .pc_relative = false };
  arelent rel = { .howto = &howto, .addend = 0, .address = 0 };

  // Pass error_message as NULL to trigger the NULL dereference in the vulnerable code path
  char **error_message = NULL;

  // output_bfd must be NULL to take the vulnerable branch
  bfd *output_bfd = NULL;

  // Call the vulnerable function; this should dereference NULL and crash
  bfd_reloc_status_type st = coff_amd64_reloc(&elf_bfd, &rel, NULL, NULL, &in_sec, output_bfd, error_message);

  // Should not reach here; but print status if it does
  printf("Reloc status: %d\n", (int)st);
  return 0;
}
