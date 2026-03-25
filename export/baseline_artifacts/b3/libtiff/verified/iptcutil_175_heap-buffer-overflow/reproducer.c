#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Minimal reconstruction of the vulnerable pieces from contrib/iptcutil/iptcutil.c
// Focus is on convertHTMLcodes() and the html_codes table it references.

typedef struct {
    int len;
    const char *code;
    char val;
} html_code;

// Only include one entry that will trigger the overlapping strcpy path.
static html_code html_codes[] = {
    {6, "&quot;", '"'}
};

#ifndef STRNICMP
#define STRNICMP strncasecmp
#endif

static int convertHTMLcodes(char *s, int len)
{
    if (len <= 0 || s == (char *)NULL || *s == '\0')
        return 0;

    if (s[1] == '#')
    {
        int val, o;

        if (sscanf(s, "&#%d;", &val) == 1)
        {
            o = 3;
            while (o < len && s[o] != ';')
            {
                o++;
                if (o > 5)
                    break;
            }
            if (o < 5 && o < len)
                strcpy(s + 1, s + 1 + o);
            *s = (char)val;
            return o;
        }
    }
    else
    {
        int i, codes = (int)(sizeof(html_codes) / sizeof(html_code));

        for (i = 0; i < codes; i++)
        {
            if (html_codes[i].len <= len)
                if (STRNICMP(s, html_codes[i].code, html_codes[i].len) == 0)
                {
                    // Vulnerable overlapping copy: dest = s+1, src = s+len (left shift)
                    strcpy(s + 1, s + html_codes[i].len);
                    *s = html_codes[i].val;
                    return html_codes[i].len - 1;
                }
        }
    }

    return 0;
}

int main(void)
{
    // Allocate a small heap buffer and intentionally do NOT NUL-terminate it,
    // so strcpy reads/writes past the end when the overlapping copy happens.
    size_t alloc = 8; // small buffer on heap
    char *buf = (char *)malloc(alloc);
    if (!buf) return 1;

    // Fill entire buffer with non-zero bytes to avoid an early NUL.
    memset(buf, 'A', alloc);

    // Place the HTML escape at the beginning without a terminating NUL in-buffer.
    // "&quot;" is 6 bytes: &, q, u, o, t, ;
    memcpy(buf, "&quot;", 6);

    // Trigger the vulnerable path: s[1] != '#', and STRNICMP matches "&quot;".
    // Pass len large enough to satisfy the html_codes[i].len <= len check.
    // Because the buffer is not NUL-terminated, strcpy(s+1, s+6) will
    // read/write beyond the heap buffer, which ASan reports as heap-buffer-overflow.
    int dropped = convertHTMLcodes(buf, 6);

    // Prevent unused warnings and keep program behavior visible.
    printf("Dropped: %d, first char now: %d\n", dropped, (int)buf[0]);

    free(buf);
    return 0;
}