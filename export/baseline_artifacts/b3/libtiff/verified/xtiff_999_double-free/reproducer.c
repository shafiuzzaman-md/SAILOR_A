#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * Self-contained stubs to mimic the relevant parts of Xlib behavior
 * needed to trigger the double-free in CreateXImage.
 */
typedef int Bool;
#define True 1
#define False 0

typedef struct XImage {
    int width;
    int height;
    char *data;
} XImage;

/* Stubs for other Xlib types used in the original code path (not actually used here) */
typedef void* Display;
typedef unsigned long Pixmap;
typedef void* GC;

typedef struct {
    Bool usePixmap;
} AppData;

static AppData appData;

/*
 * XCreateImage stub: create an XImage and set its data pointer to the caller-supplied buffer.
 * This mirrors the real XCreateImage semantics that store the data pointer in the structure.
 */
XImage* XCreateImage(Display *dpy, void *visual, unsigned int depth, int format,
                     int offset, char *data, unsigned int width, unsigned int height,
                     int bitmap_pad, int bytes_per_line)
{
    (void)dpy; (void)visual; (void)depth; (void)format; (void)offset; (void)bitmap_pad; (void)bytes_per_line;
    XImage *img = (XImage*)malloc(sizeof(XImage));
    if (!img) return NULL;
    img->width = (int)width;
    img->height = (int)height;
    img->data = data; /* crucial: store the provided buffer pointer */
    return img;
}

/*
 * XDestroyImage stub: frees both the image->data buffer and the XImage structure itself.
 * This matches Xlib's documented behavior that leads to the double-free in the original code.
 */
void XDestroyImage(XImage *img)
{
    if (!img) return;
    if (img->data) {
        free(img->data);
        img->data = NULL;
    }
    free(img);
}

/*
 * Minimal CreateXImage replica that exercises the buggy path:
 * when appData.usePixmap is True, it destroys the XImage and then frees imageMemory again.
 */
static void CreateXImage_bug(XImage *xImage, char *imageMemory, int xImageDepth)
{
    /* The surrounding X operations are irrelevant to the bug, so we omit them. */
    (void)xImageDepth; /* unused in this minimal reproducer */

    if (appData.usePixmap == True) {
        /* In the original code, XDestroyImage frees both xImage and xImage->data. */
        XDestroyImage(xImage);
        /* Double free bug: imageMemory == xImage->data, so this frees the same pointer again. */
        free(imageMemory);
    }
}

int main(void)
{
    /* Prepare a buffer that will be owned by the XImage. */
    size_t buf_size = 1024;
    char *imageMemory = (char*)malloc(buf_size);
    if (!imageMemory) {
        perror("malloc");
        return 1;
    }
    memset(imageMemory, 0xAA, buf_size);

    /* Create an XImage whose data pointer is imageMemory, mirroring XCreateImage behavior. */
    int depth = 24; /* arbitrary */
    XImage *xImage = XCreateImage(NULL, NULL, (unsigned)depth, /*format*/2, /*offset*/0,
                                  imageMemory, /*width*/16, /*height*/16, /*bitmap_pad*/32, /*bytes_per_line*/0);
    if (!xImage) {
        fprintf(stderr, "XCreateImage failed\n");
        free(imageMemory);
        return 1;
    }

    /* Set the flag to take the buggy branch. */
    appData.usePixmap = True;

    /* Trigger the double free: XDestroyImage frees imageMemory, then we free it again. */
    CreateXImage_bug(xImage, imageMemory, depth);

    /* If the bug didn't crash already, return normally. ASan should report double free. */
    return 0;
}