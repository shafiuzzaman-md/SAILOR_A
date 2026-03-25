#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* Copied from contrib/libtests/pngvalid.c */
static size_t
safecat(char *buffer, size_t bufsize, size_t pos, const char *cat)
{
    while (pos < bufsize && cat != NULL && *cat != 0)
        buffer[pos++] = *cat++;

    if (pos >= bufsize)
        pos = bufsize-1;  /* Underflows to SIZE_MAX when bufsize == 0 */

    buffer[pos] = 0;      /* Out-of-bounds write when bufsize == 0 */
    return pos;
}

int main(void)
{
    /* Allocate a 1-byte heap buffer so the write to buffer[-1] triggers
     * a heap-buffer-underflow detectable by ASan.
     */
    char *buf = (char*)malloc(1);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }

    /* Trigger the bug: pass bufsize == 0 so safecat sets pos = bufsize-1,
     * which underflows to SIZE_MAX, then writes buffer[pos] = 0.
     * On two's complement architectures this is effectively buffer[-1].
     */
    size_t ret = safecat(buf, 0 /* bufsize */, 0 /* pos */, "A");

    /* Prevent dead-code elimination and ensure side effects are observable. */
    printf("safecat returned: %zu\n", ret);

    free(buf);
    return 0;
}
