// Standalone reproducer for OOB read in extract_long_section_name
// CWE-125: Out-of-bounds Read
// This program stubs minimal parts of BFD needed to call the vulnerable function
// and crafts a string table without a NUL terminator at the checked offset.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef size_t bfd_size_type;

typedef struct bfd {
    char *string_table;
    bfd_size_type string_table_len;
} bfd;

// Stubs for the BFD helpers used by extract_long_section_name
static const char * _bfd_coff_read_string_table(bfd *abfd) {
    return abfd->string_table;
}

static bfd_size_type obj_coff_strings_len(bfd *abfd) {
    return abfd->string_table_len;
}

static void * bfd_alloc(bfd *abfd, bfd_size_type size) {
    (void)abfd; // unused in this stub
    return malloc(size);
}

// Vulnerable function copied/adapted from bfd/coffgen.c
static char *
extract_long_section_name(bfd *abfd, unsigned long strindex)
{
    const char *strings;
    char *name;

    strings = _bfd_coff_read_string_table (abfd);
    if (strings == NULL)
        return NULL;
    if ((bfd_size_type)(strindex + 2) >= obj_coff_strings_len (abfd))
        return NULL;
    strings += strindex;
    // BUG: strlen can run past the end of the string table if no NUL is present
    name = (char *) bfd_alloc (abfd, (bfd_size_type) strlen (strings) + 1);
    if (name == NULL)
        return NULL;
    strcpy (name, strings);

    return name;
}

int main(void) {
    // Create a fake BFD with a string table that lacks a NUL terminator
    // at and after the chosen strindex. This satisfies the bounds check
    // (strindex + 2) < table_length but forces strlen to read past the end.
    bfd abfd;

    const bfd_size_type table_len = 32; // any length >= 3
    abfd.string_table = (char *)malloc(table_len);
    if (!abfd.string_table) {
        perror("malloc");
        return 1;
    }
    abfd.string_table_len = table_len;

    // Fill the table with non-NUL bytes to ensure no terminator is present
    memset(abfd.string_table, 'A', table_len);

    // Choose an index that passes the check: (strindex + 2) < table_len
    // For table_len=32, strindex=29 satisfies 29+2=31 < 32
    unsigned long strindex = table_len - 3; // 29

    fprintf(stderr, "Triggering OOB read: table_len=%zu, strindex=%lu\n",
            (size_t)table_len, strindex);

    // This call should trigger an ASan out-of-bounds read in strlen
    char *name = extract_long_section_name(&abfd, strindex);

    // If the bug did not trigger (e.g., ASan not enabled), clean up
    if (name) free(name);
    free(abfd.string_table);

    return 0;
}
