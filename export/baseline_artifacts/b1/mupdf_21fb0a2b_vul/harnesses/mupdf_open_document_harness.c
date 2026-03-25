/*
 * B1 Human-written harness: mupdf document open/parse path
 * Targets: fz_open_document_with_buffer, fz_count_pages, fz_load_page
 * Modeled after OSS-Fuzz pdf_fuzzer
 */
#include <mupdf/fitz.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_BUF_SIZE 256

int main(void) {
    unsigned char data[MAX_BUF_SIZE];
    klee_make_symbolic(data, sizeof(data), "pdf_input");

    /* Require PDF signature */
    data[0] = '%';
    data[1] = 'P';
    data[2] = 'D';
    data[3] = 'F';
    data[4] = '-';

    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    if (!ctx) return 0;

    fz_try(ctx) {
        fz_register_document_handlers(ctx);

        fz_buffer *buf = fz_new_buffer_from_copied_data(ctx, data, sizeof(data));

        fz_document *doc = fz_open_document_with_buffer(ctx, "application/pdf", buf);

        int pages = fz_count_pages(ctx, doc);
        if (pages > 0 && pages < 10) {
            fz_page *page = fz_load_page(ctx, doc, 0);
            fz_rect bounds = fz_bound_page(ctx, page);
            (void)bounds;
            fz_drop_page(ctx, page);
        }

        fz_drop_document(ctx, doc);
        fz_drop_buffer(ctx, buf);
    }
    fz_catch(ctx) {
        /* Expected — invalid PDF data */
    }

    fz_drop_context(ctx);
    return 0;
}
