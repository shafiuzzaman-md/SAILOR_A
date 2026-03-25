// Standalone C reproducer for NULL pointer dereference in coff_link_add_symbols
// Trigger: section == NULL but coff_section_data(abfd, section) is evaluated
// Compile: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Minimal stub types to simulate BFD/COFF environment
typedef struct bfd bfd;
typedef struct asection asection;

typedef struct coff_comdat {
  const char *name;
} coff_comdat;

typedef struct coff_section_tdata {
  coff_comdat *comdat;
} coff_section_tdata;

struct asection {
  // In real BFD, COFF per-section data is hung off the section; we model that here.
  coff_section_tdata *tdata;
};

struct bfd {
  int dummy;
};

// Constants that mirror the relevant COFF symbol classifications
#define COFF_SYMBOL_GLOBAL 1
#define COFF_SYMBOL_PE_SECTION 2

// Macro mirroring how BFD accesses per-section COFF data. This dereferences 'sec'.
#define coff_section_data(abfd, sec) ((sec)->tdata)

// obj_pe(abfd) predicate stub: force PE to be true to enter the vulnerable path.
static int obj_pe(bfd *abfd) {
  (void)abfd;
  return 1; // Simulate a PE object file
}

// startswith helper similar to what's used in BFD sources
static int startswith(const char *s, const char *prefix) {
  size_t n = strlen(prefix);
  return strncmp(s, prefix, n) == 0;
}

// Stub for the function that maps a COFF section index to a section pointer.
// For an invalid/malformed index, it returns NULL (the crux of the bug scenario).
static asection *coff_section_from_bfd_index(bfd *abfd, int idx) {
  (void)abfd; (void)idx;
  return NULL; // Simulate malformed/invalid section index -> no section found
}

// Vulnerable function stub mirroring the problematic conditional from bfd/cofflink.c
// We set up local variables to drive evaluation into the vulnerable condition.
bool coff_link_add_symbols(bfd *abfd, void *info) {
  (void)info;

  // Simulate a symbol that will be treated as GLOBAL or PE_SECTION, with a name starting with "??_"
  int classification = COFF_SYMBOL_GLOBAL; // or COFF_SYMBOL_PE_SECTION
  const char *name = "??_BadComdatName";

  // Critical: section is NULL as returned for an invalid section index
  asection *section = coff_section_from_bfd_index(abfd, 0x7fffffff);

  // This condition sequence matches the vulnerable block in cofflink.c (lines ~428-435).
  // It evaluates coff_section_data(abfd, section) without checking that 'section' is non-NULL.
  if (obj_pe(abfd)
      && (classification == COFF_SYMBOL_GLOBAL || classification == COFF_SYMBOL_PE_SECTION)
      && coff_section_data(abfd, section) != NULL  // NULL dereference occurs here because 'section' is NULL
      && coff_section_data(abfd, section)->comdat != NULL
      && startswith(name, "??_")
      && strcmp(name, coff_section_data(abfd, section)->comdat->name) == 0) {
    // We will never reach here due to the crash above.
    printf("This should not print.\n");
  }

  return true;
}

int main(void) {
  // Create a dummy BFD object; contents are irrelevant for this reproducer.
  bfd *abfd = (bfd *)calloc(1, sizeof(bfd));
  if (!abfd) {
    fprintf(stderr, "Allocation failed\n");
    return 1;
  }

  // Calling the vulnerable function with crafted state triggers the NULL dereference
  // when evaluating coff_section_data(abfd, section) with section == NULL.
  (void)coff_link_add_symbols(abfd, NULL);

  free(abfd);
  return 0;
}
