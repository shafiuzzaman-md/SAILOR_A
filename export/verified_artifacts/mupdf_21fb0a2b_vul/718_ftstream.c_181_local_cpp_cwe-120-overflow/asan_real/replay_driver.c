#include <string.h>
// NO_HARNESS_TYPES
#include <stddef.h>
// klee removed for replay

/* Minimal FreeType-like type defs matching harness/ftstream.c */
typedef unsigned char  FT_Byte;
typedef unsigned long  FT_ULong;
typedef long           FT_Long;

struct FT_StreamRec_;
typedef struct FT_StreamRec_* FT_Stream;

typedef FT_ULong (*FT_Stream_IoFunc)(FT_Stream stream,
                                     FT_ULong  offset,
                                     FT_Byte*  buffer,
                                     FT_ULong  count);

typedef struct FT_StreamRec_ {
    FT_Byte*         base;
    FT_ULong         size;
    FT_ULong         pos;
    FT_Stream_IoFunc read;
} FT_StreamRec;

/* Allocators (avoid pulling system headers that might conflict) */
extern void *malloc(size_t);
extern void *calloc(size_t, size_t);

#ifndef SRC_SIZE
#define SRC_SIZE 64
#endif
#ifndef DST_SIZE
#define DST_SIZE 8
#endif

extern int FT_Stream_TryRead( FT_Stream stream, FT_Byte* buffer, FT_ULong count );

int main() {
    // Allocate and initialize FT_StreamRec
    FT_StreamRec *s = (FT_StreamRec *)calloc(1, sizeof(FT_StreamRec));
    if (!s) return 0;

    // Source buffer backing the stream
    FT_Byte *src = (FT_Byte *)malloc(SRC_SIZE);
    if (!src) return 0;
    { static const unsigned char stream_base_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, stream_base_data, (SRC_SIZE < sizeof(stream_base_data)) ? SRC_SIZE : sizeof(stream_base_data)); };

    s->base = src;
    s->size = SRC_SIZE;
    s->pos  = 0;           // start at beginning
    s->read = 0;           // force else-branch with FT_MEM_COPY

    // Destination buffer that may overflow
    FT_Byte *dst = (FT_Byte *)malloc(DST_SIZE);
    if (!dst) return 0;
    { static const unsigned char dst_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(dst, dst_buf_data, (DST_SIZE < sizeof(dst_buf_data)) ? DST_SIZE : sizeof(dst_buf_data)); };

    // Symbolic count to control copy size
    FT_ULong count;
    { static const unsigned char count_data[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&count, count_data, (sizeof(count) < sizeof(count_data)) ? sizeof(count) : sizeof(count_data)); };
    // Keep within source bounds and bias towards overflow over dst
    /* klee_assume removed */
    /* klee_assume removed */ // encourage overflow into dst

    // Call entry (directly calls vulnerable function)
    FT_Stream_TryRead(s, dst, count);
    return 0;
}
