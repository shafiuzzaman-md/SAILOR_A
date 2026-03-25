// Standalone reproducer for NULL pointer dereference in png_convert_to_rfc1123_buffer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libpng-like typedefs */
typedef unsigned char png_byte;
typedef unsigned short png_uint_16;

typedef struct png_time_s {
    png_uint_16 year;   /* full year, e.g., 2025 */
    png_byte month;     /* 1..12 */
    png_byte day;       /* 1..31 */
    png_byte hour;      /* 0..23 */
    png_byte minute;    /* 0..59 */
    png_byte second;    /* 0..60 (leap second) */
} png_time;

#define PNG_UNUSED(x) (void)(x)

/* Minimal helpers to satisfy references in the vulnerable function */
static size_t png_safecat(char *out, size_t outsize, size_t pos, const char *str)
{
    if (out == NULL || outsize == 0) return pos;
    while (pos + 1 < outsize && *str) {
        out[pos++] = *str++;
    }
    if (pos < outsize) out[pos] = '\0';
    return pos;
}

#define PNG_NUMBER_FORMAT_u    1
#define PNG_NUMBER_FORMAT_02u  2

static const char* png_format_number(char *buf, unsigned fmt, unsigned value)
{
    if (fmt == PNG_NUMBER_FORMAT_02u)
        snprintf(buf, 5, "%02u", value);
    else
        snprintf(buf, 5, "%u", value);
    return buf;
}

#define PNG_FORMAT_NUMBER(buf, fmt, val) png_format_number((buf), (fmt), (val))

/* Vulnerable function copied/minimized from the source context */
int png_convert_to_rfc1123_buffer(char out[29], const png_time *ptime)
{
    static const char short_months[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    if (out == NULL)
        return 0;

    /* Vulnerability: ptime is dereferenced without NULL check */
    if (ptime->year > 9999 /* RFC1123 limitation */ ||
        ptime->month == 0    ||  ptime->month > 12  ||
        ptime->day   == 0    ||  ptime->day   > 31  ||
        ptime->hour  > 23    ||  ptime->minute > 59 ||
        ptime->second > 60)
        return 0;

    {
        size_t pos = 0;
        char number_buf[5] = {0, 0, 0, 0, 0}; /* enough for a four-digit year */

#       define APPEND_STRING(string) pos = png_safecat(out, 29, pos, (string))
#       define APPEND_NUMBER(format, value) \
            APPEND_STRING(PNG_FORMAT_NUMBER(number_buf, format, (value)))
#       define APPEND(ch) if (pos < 28) out[pos++] = (ch)

        APPEND_NUMBER(PNG_NUMBER_FORMAT_u, (unsigned)ptime->day);
        APPEND(' ');
        APPEND_STRING(short_months[(ptime->month - 1)]);
        APPEND(' ');
        APPEND_NUMBER(PNG_NUMBER_FORMAT_u, ptime->year);
        APPEND(' ');
        APPEND_NUMBER(PNG_NUMBER_FORMAT_02u, (unsigned)ptime->hour);
        APPEND(':');
        APPEND_NUMBER(PNG_NUMBER_FORMAT_02u, (unsigned)ptime->minute);
        APPEND(':');
        APPEND_NUMBER(PNG_NUMBER_FORMAT_02u, (unsigned)ptime->second);
        APPEND_STRING(" +0000"); /* This reliably terminates the buffer */
        PNG_UNUSED(pos)

#       undef APPEND
#       undef APPEND_NUMBER
#       undef APPEND_STRING
    }

    return 1;
}

int main(void)
{
    char out[29];
    memset(out, 0, sizeof(out));

    /* Trigger: pass NULL for ptime to cause NULL dereference at ptime->year */
    (void)png_convert_to_rfc1123_buffer(out, NULL);

    /* If it didn't crash (it should), print the buffer to avoid optimization */
    printf("out: %s\n", out);
    return 0;
}
