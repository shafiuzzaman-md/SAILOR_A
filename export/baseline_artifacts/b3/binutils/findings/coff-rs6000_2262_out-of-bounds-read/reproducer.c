// Self-contained reproducer for OOB read in xcoff_write_armap_big
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Minimal stand-ins for BFD structures used by the vulnerable code
typedef struct bfd bfd;

typedef struct bfd_arch_info {
    int bits_per_address; // we only need this
} bfd_arch_info;

struct bfd {
    bfd *archive_head;
    bfd *archive_next;
};

// Minimal version of the ORL entry used by the vulnerable code
struct orl {
    bfd *abfd;          // member bfd this symbol belongs to
    const char **name;  // pointer to a string pointer (matches *map[i].name)
};

// Stub: always return some architecture info.
static const bfd_arch_info *bfd_get_arch_info(bfd *abfd) {
    static bfd_arch_info info = { 32 }; // 32 is enough to trigger the bug while skipping sprintf path
    (void)abfd;
    return &info;
}

// Vulnerable function (trimmed to the essential buggy loop)
static bool xcoff_write_armap_big(bfd *abfd, unsigned int elength, struct orl *map, unsigned int orl_count, int stridx) {
    (void)elength;
    (void)stridx;

    // Dummy variables to satisfy references in the original code
    char dummy_buf[16];
    char *st = dummy_buf;
    int string_length = 0;

    const bfd_arch_info *arch_info;
    unsigned int i = 0;
    bfd *current_bfd;

    // This reproduces the buggy 64-bit symbol names loop from the source.
    for (current_bfd = abfd->archive_head;
         current_bfd != NULL && i < orl_count;
         current_bfd = current_bfd->archive_next) {
        arch_info = bfd_get_arch_info(current_bfd);
        // BUG: no bounds check for i against orl_count inside the while condition
        while (map[i].abfd == current_bfd) {
            if (arch_info->bits_per_address == 64) {
                // Not executed with our stub (bits_per_address == 32), but keep for fidelity
                string_length = sprintf(st, "%s", *map[i].name);
                st += string_length + 1;
            }
            i++;
        }
    }

    return true;
}

int main(void) {
    // Build a minimal archive with a single member
    bfd archive = {0};
    bfd memberA = {0};

    archive.archive_head = &memberA;
    memberA.archive_next = NULL;

    // Create an orl map with 3 entries, all pointing to memberA.
    // This ensures that inside the while loop, i will be incremented from 0 to 3
    // and then the condition will be re-evaluated at i == orl_count, causing an OOB read.
    const unsigned int orl_count = 3;
    struct orl *map = (struct orl *)malloc(orl_count * sizeof(*map));
    if (!map) {
        perror("malloc");
        return 1;
    }

    const char *names[orl_count];
    names[0] = "sym0";
    names[1] = "sym1";
    names[2] = "sym2";

    for (unsigned int i = 0; i < orl_count; i++) {
        map[i].abfd = &memberA;   // All entries belong to the same member
        map[i].name = &names[i];  // Provide valid names (not used in our path)
    }

    // Call the vulnerable function; ASan should report a heap-buffer-overflow (OOB read)
    (void)xcoff_write_armap_big(&archive, 0, map, orl_count, 0);

    // Clean up (won't be reached if ASan aborts on error)
    free(map);
    return 0;
}
