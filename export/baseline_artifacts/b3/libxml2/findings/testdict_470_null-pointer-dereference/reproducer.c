#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* Minimal libxml2-like typedefs and API stubs */
typedef unsigned char xmlChar;

/* Simulate allocation failure: xmlMalloc returns NULL */
void *xmlMalloc(size_t size) {
    (void)size;
    return NULL; /* Force failure to trigger NULL dereference */
}

void xmlFree(void *ptr) {
    free(ptr);
}

/* From the provided source context */
#define ATTRIBUTE_NO_SANITIZE_INTEGER
#define HASH_ROL(x,n) (((x) << (n)) | (((x) & 0xFFFFFFFFu) >> (32 - (n))))

static unsigned rng_state[2] = { 123, 456 };

ATTRIBUTE_NO_SANITIZE_INTEGER
static unsigned my_rand(unsigned max) {
    unsigned s0 = rng_state[0];
    unsigned s1 = rng_state[1];
    unsigned result = HASH_ROL(s0 * 0x9E3779BBu, 5) * 5u;

    s1 ^= s0;
    rng_state[0] = HASH_ROL(s0, 26) ^ s1 ^ (s1 << 9);
    rng_state[1] = HASH_ROL(s1, 13);

    /* In the original code, max is always > 0 */
    return ((result & 0xFFFFFFFFu) % max);
}

static xmlChar *
gen_random_string(xmlChar id) {
    unsigned size = my_rand(64) + 1;   /* size in [1,64] */
    unsigned id_pos = my_rand(size);   /* position within size */
    size_t j;

    xmlChar *str = (xmlChar *)xmlMalloc(size + 1);
    /* BUG: str may be NULL (from xmlMalloc failure), but it's dereferenced unconditionally */
    for (j = 0; j < size; j++) {
        str[j] = (xmlChar)('a' + my_rand(26)); /* NULL dereference here if str == NULL */
    }
    str[id_pos] = id;
    str[size] = 0;

    /* Generate QName in 75% of cases */
    if (size > 3 && my_rand(4) > 0) {
        unsigned colon_pos = my_rand(size - 3) + 1;

        if (colon_pos >= id_pos)
            colon_pos++;
        str[colon_pos] = ':';
    }

    return str;
}

int main(void) {
    /* Calling gen_random_string will attempt to write to str even if allocation failed */
    xmlChar id = (xmlChar)'X';
    (void)gen_random_string(id);
    /* We should never reach here; ASan should report a NULL pointer dereference */
    return 0;
}
