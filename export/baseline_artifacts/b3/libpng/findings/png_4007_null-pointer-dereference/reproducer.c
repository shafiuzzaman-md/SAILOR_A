#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libpng types/macros used by the vulnerable code */
typedef char* png_charp;
typedef const char* png_const_charp;

#ifndef PNG_IMAGE_ERROR
#define PNG_IMAGE_ERROR 2
#endif

/* Minimal stand-in for png_image that includes only the fields used */
typedef struct png_image {
    unsigned int warning_or_error;
    char message[64];
    void *opaque;
} png_image;

/* Stub of png_image_free as referenced by png_image_error */
void png_image_free(png_image *image)
{
    /* Safe no-op; real implementation would free internal state. */
    (void)image;
}

/* Minimal png_safecat implementation matching expected behavior */
static size_t png_safecat(png_charp buffer, size_t bufsize, size_t pos, png_const_charp string)
{
    size_t len = strlen(string);
    if (pos < bufsize) {
        size_t avail = (bufsize > pos) ? (bufsize - pos - 1) : 0; /* reserve for NUL */
        if (len > avail) len = avail;
        if (len > 0) memcpy(buffer + pos, string, len);
        pos += len;
        if (pos < bufsize) buffer[pos] = '\0';
    }
    return pos;
}

/* Vulnerable function (mirrors the bug: unconditional dereference of image) */
int png_image_error(png_image *image, const char *error_message)
{
    /* Utility to log an error. */
    png_safecat(image->message, (sizeof image->message), 0, error_message);
    image->warning_or_error |= PNG_IMAGE_ERROR;
    png_image_free(image);
    return 0;
}

int main(void)
{
    /* Trigger: pass a NULL image pointer, causing immediate NULL dereference
       at image->message inside png_image_error via png_safecat. */
    const char *msg = "Triggering null-pointer dereference in png_image_error";
    /* This call will crash with ASan reporting a null-pointer dereference. */
    (void)png_image_error(NULL, msg);

    /* Should never reach here */
    puts("If you see this, the bug did not trigger as expected.");
    return 0;
}
