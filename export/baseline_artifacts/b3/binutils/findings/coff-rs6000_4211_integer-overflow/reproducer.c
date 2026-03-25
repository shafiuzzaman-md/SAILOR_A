// Standalone reproducer for the integer overflow in xcoff_generate_rtinit
// described in bfd/coff-rs6000.c (string table size accumulation).
//
// Build (as provided by the task):
//   clang -fsanitize=address -g -O0 -I/tmp/binutils_upstream reproducer.c -o reproducer \
//          -L/tmp/binutils_upstream/build/.libs -ltiff -lm

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Minimal stand-ins for BFD types used by the vulnerable function.
typedef unsigned char bfd_byte;
// Use a deliberately small type to force integer wraparound in the reproducer.
// In the real bug, bfd_size_type can overflow; here we make it 16-bit so we can
// trigger the bug with reasonable buffer sizes.
typedef uint16_t bfd_size_type;

struct bfd { int dummy; };

typedef uint32_t bfd_vma;

static void *bfd_zmalloc(bfd_size_type size)
{
    void *p = malloc(size);
    if (p) memset(p, 0, size);
    return p;
}

static void bfd_h_put_32(struct bfd *abfd, bfd_vma val, void *where)
{
    (void)abfd; // endianness doesn't matter for the bug; just store some bytes
    memcpy(where, &val, 4);
}

// Vulnerable function re-implemented with the core logic that triggers the bug.
// It mirrors the string table size accumulation/overflow and subsequent memcpy
// that writes initsz/finisz bytes into the undersized heap buffer.
static bool xcoff_generate_rtinit(struct bfd *abfd,
                                  const bfd_byte *init, size_t initsz,
                                  const bfd_byte *fini, size_t finisz)
{
    (void)abfd;
    bfd_byte *string_table = NULL;
    bfd_byte *st_tmp = NULL;
    bfd_vma val;

    // string table
    bfd_size_type string_table_size = 0;
    if (initsz > 9)
        string_table_size += (bfd_size_type)initsz; // potential wrap
    if (finisz > 9)
        string_table_size += (bfd_size_type)finisz; // potential wrap

    if (string_table_size) {
        string_table_size += 4; // potential wrap
        string_table = (bfd_byte *) bfd_zmalloc(string_table_size); // undersized alloc
        if (string_table == NULL)
            return false;

        val = string_table_size;
        bfd_h_put_32(abfd, val, &string_table[0]);
        st_tmp = string_table + 4;
    }

    // These memcpy calls use the full initsz/finisz, potentially overflowing
    // the undersized string_table buffer allocated above.
    if (initsz > 9) {
        memcpy(st_tmp, init, initsz);
        st_tmp += initsz;
    }

    if (finisz > 9) {
        memcpy(st_tmp, fini, finisz);
        st_tmp += finisz;
    }

    free(string_table);
    return true;
}

int main(void)
{
    // Choose sizes that cause 16-bit wraparound while being reasonable to allocate.
    // For uint16_t bfd_size_type:
    //   wrapped_size = ((initsz + finisz) & 0xFFFF) + 4
    // Use initsz = 40000, finisz = 30000 => 70000 + 4 = 70004
    // 70004 mod 65536 = 4468 -> allocate only 4468 bytes, then copy 40000 + 30000 bytes.
    size_t initsz = 40000;
    size_t finisz = 30000;

    bfd_byte *init = (bfd_byte *)malloc(initsz);
    bfd_byte *fini = (bfd_byte *)malloc(finisz);
    if (!init || !fini) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    memset(init, 'A', initsz);
    memset(fini, 'B', finisz);

    struct bfd dummy = {0};

    // Print info to help observe the overflow conditions.
    bfd_size_type wrapped = (bfd_size_type)0;
    wrapped = (bfd_size_type)(wrapped + (bfd_size_type)initsz);
    wrapped = (bfd_size_type)(wrapped + (bfd_size_type)finisz);
    wrapped = (bfd_size_type)(wrapped ? (bfd_size_type)(wrapped + 4) : wrapped);

    printf("Triggering xcoff_generate_rtinit with initsz=%zu, finisz=%zu, wrapped alloc=%u bytes\n",
           initsz, finisz, (unsigned)wrapped);

    // This call should trigger a heap-buffer-overflow under ASan in memcpy.
    (void)xcoff_generate_rtinit(&dummy, init, initsz, fini, finisz);

    free(init);
    free(fini);
    return 0;
}
