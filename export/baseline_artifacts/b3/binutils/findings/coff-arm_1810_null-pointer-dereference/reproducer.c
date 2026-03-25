#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* Minimal stand-ins for BFD types */
typedef unsigned char bfd_byte;
typedef uintptr_t bfd_vma;

typedef struct asection asection;
typedef struct bfd bfd;

struct asection {
    const char *name;
    bfd_byte *contents;
    size_t size;
    int alloced;
    bfd_vma vma;
    asection *next;
    bfd *owner;
};

struct bfd {
    asection *sections;
};

struct coff_arm_link_hash_table {
    size_t arm_glue_size;
    size_t thumb_glue_size;
    bfd *bfd_of_glue_owner;
};

struct bfd_link_info {
    void *callbacks; /* unused here */
    void *hash;      /* we store our coff_arm_link_hash_table* here */
};

/* Names used by coff-arm.c for interworking glue sections */
#define ARM2THUMB_GLUE_SECTION_NAME ".glue_7"
#define THUMB2ARM_GLUE_SECTION_NAME ".glue_7t"

/* Emulate BFD_ASSERT */
#define BFD_ASSERT(x) assert(x)

/* Minimal helpers mimicking BFD API used by the vulnerable function */
static asection *
bfd_get_section_by_name(bfd *abfd, const char *name)
{
    for (asection *s = abfd->sections; s; s = s->next) {
        if (s->name && strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

/* Stub bfd_alloc that simulates allocation failure by returning NULL */
static bfd_byte *
bfd_alloc(bfd *abfd, size_t size)
{
    (void)abfd; (void)size;
    return NULL; /* Force failure to trigger the bug */
}

/* coff_arm_hash_table accessor used by bfd_arm_allocate_interworking_sections */
static struct coff_arm_link_hash_table *
coff_arm_hash_table(struct bfd_link_info *info)
{
    return (struct coff_arm_link_hash_table *)info->hash;
}

/* Vulnerable function copied/abstracted from bfd/coff-arm.c */
bool bfd_arm_allocate_interworking_sections(struct bfd_link_info *info)
{
    asection *s;
    bfd_byte *foo;
    struct coff_arm_link_hash_table *globals;

    globals = coff_arm_hash_table(info);
    BFD_ASSERT(globals != NULL);

    if (globals->arm_glue_size != 0) {
        BFD_ASSERT(globals->bfd_of_glue_owner != NULL);
        s = bfd_get_section_by_name(globals->bfd_of_glue_owner, ARM2THUMB_GLUE_SECTION_NAME);
        BFD_ASSERT(s != NULL);
        foo = bfd_alloc(globals->bfd_of_glue_owner, globals->arm_glue_size); /* May return NULL */
        s->size = globals->arm_glue_size;
        s->contents = foo; /* Assigned without NULL check */
        s->alloced = 1;
    }

    if (globals->thumb_glue_size != 0) {
        BFD_ASSERT(globals->bfd_of_glue_owner != NULL);
        s = bfd_get_section_by_name(globals->bfd_of_glue_owner, THUMB2ARM_GLUE_SECTION_NAME);
        BFD_ASSERT(s != NULL);
        foo = bfd_alloc(globals->bfd_of_glue_owner, globals->thumb_glue_size);
        s->size = globals->thumb_glue_size;
        s->contents = foo;
        s->alloced = 1;
    }

    return true;
}

/* Minimal bfd_put_32 that writes into target buffer (will NULL-deref if buffer is NULL) */
static inline void bfd_put_32(bfd *abfd, uint32_t v, bfd_byte *where)
{
    (void)abfd;
    /* Little-endian write as used by typical ARM targets */
    where[0] = (bfd_byte)(v & 0xFF);
    where[1] = (bfd_byte)((v >> 8) & 0xFF);
    where[2] = (bfd_byte)((v >> 16) & 0xFF);
    where[3] = (bfd_byte)((v >> 24) & 0xFF);
}

/* Simulate later code paths in coff-arm.c that write into s->contents */
static void simulate_glue_emission(bfd *abfd)
{
    asection *s = bfd_get_section_by_name(abfd, ARM2THUMB_GLUE_SECTION_NAME);
    BFD_ASSERT(s != NULL);
    /* This mimics the glue emission using bfd_put_16/32 in coff-arm.c */
    bfd_put_32(abfd, 0xE12FFF1E, s->contents); /* Will NULL-deref if contents is NULL */
}

int main(void)
{
    /* Build a fake BFD with the expected glue sections present */
    bfd *owner = (bfd *)calloc(1, sizeof(bfd));
    if (!owner) return 1;

    asection *arm2thumb = (asection *)calloc(1, sizeof(asection));
    if (!arm2thumb) return 1;
    arm2thumb->name = ARM2THUMB_GLUE_SECTION_NAME;
    arm2thumb->owner = owner;
    arm2thumb->next = NULL;
    owner->sections = arm2thumb;

    /* Prepare the globals/hash table with a non-zero glue size */
    struct coff_arm_link_hash_table globals;
    memset(&globals, 0, sizeof(globals));
    globals.arm_glue_size = 16;   /* Non-zero to enter allocation path */
    globals.thumb_glue_size = 0;  /* Not used in this reproducer */
    globals.bfd_of_glue_owner = owner;

    /* Link info pointing to our globals */
    struct bfd_link_info info;
    memset(&info, 0, sizeof(info));
    info.hash = &globals;

    /* This will set s->contents = NULL due to our failing bfd_alloc */
    (void)bfd_arm_allocate_interworking_sections(&info);

    /* Later code writes into s->contents, causing a NULL pointer dereference */
    simulate_glue_emission(owner);

    /* Should not reach here */
    printf("If you see this, the bug did not trigger.\n");
    return 0;
}
