// NO_HARNESS_TYPES
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Minimal local types matching harness/pdf-xref.c
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_buffer { unsigned char *data; size_t len; } fz_buffer;
typedef struct pdf_obj { int dummy; } pdf_obj;

typedef struct pdf_xref_entry {
    int type;
    int ofs;
    int gen;
    int num;
    int stm_ofs;
    struct fz_buffer *stm_buf;
    struct pdf_obj *obj;
} pdf_xref_entry;

typedef struct pdf_xref_subsec {
    pdf_xref_entry *table;
    int start;
    int len;
    struct pdf_xref_subsec *next;
} pdf_xref_subsec;

typedef struct pdf_xref {
    struct pdf_xref_subsec *subsec;
    int num_objects;
} pdf_xref;

typedef struct pdf_document {
    struct pdf_xref xref;
} pdf_document;

// Entry prototype from harness
extern void pdf_update_stream(fz_context *ctx, pdf_document *doc, pdf_obj *obj, fz_buffer *newbuf, int compressed);

int main() {
    fz_context *ctx = (fz_context*)calloc(1, sizeof(fz_context));
    pdf_document *doc = (pdf_document*)calloc(1, sizeof(pdf_document));
    pdf_obj *obj = (pdf_obj*)calloc(1, sizeof(pdf_obj));
    fz_buffer *buf = (fz_buffer*)calloc(1, sizeof(fz_buffer));

    // Set up xref and subsec so that resize_xref_sub() loop runs
    pdf_xref_subsec *sub = (pdf_xref_subsec*)calloc(1, sizeof(pdf_xref_subsec));

    int oldlen = 2;
    sub->len = oldlen;
    sub->start = 0;
    sub->next = NULL;

    sub->table = (pdf_xref_entry*)calloc((size_t)oldlen, sizeof(pdf_xref_entry));

    doc->xref.subsec = sub;
    doc->xref.num_objects = oldlen; // base 0

    int newlen;
    { static const unsigned char newlen_data[] = {0x03, 0x00, 0x00, 0x00}; memcpy(&newlen, newlen_data, (sizeof(newlen) < sizeof(newlen_data)) ? sizeof(newlen) : sizeof(newlen_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    pdf_update_stream(ctx, doc, obj, buf, newlen);
    return 0;
}
