// Standalone reproducer for OOB-read in xtiff.c: EventProc Expose handling
// Compilable with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer \
//         -I/tmp/libtiff_upstream -L/tmp/libtiff_upstream/build/.libs -ltiff -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal re-declarations to avoid pulling X11/Xt headers
typedef void* Widget;
typedef void* Display;
typedef unsigned long Drawable;
typedef void* GC;

typedef struct {
    int width;
    int height;
    unsigned char* data;
} XImage;

// Minimal XEvent representation needed for this reproducer
typedef struct {
    int type;
    struct { int x, y, width, height; } xexpose;
    struct { int x, y; } xmotion;
    struct { int x, y; } xbutton;
} XEvent;

// Event type constants (values are arbitrary for our stubs)
enum { Expose = 12, MotionNotify = 6, ButtonRelease = 5 };

// Stubs for Xt/Xlib APIs used by the vulnerable code path
static Drawable XtWindow(Widget w) { (void)w; return 0; }

// Globals mimicking those used by xtiff.c
static struct { int usePixmap; } appData = { 0 }; // usePixmap == False
static Display* xDisplay = NULL;
static GC xWinGc = NULL;
static XImage* xImage = NULL;

static int xOffset = 0;
static int yOffset = 0;
static unsigned int tfImageWidth = 0;
static unsigned int tfImageHeight = 0;
static int iw = 0; // image width (copied from tfImageWidth)
static int ih = 0; // image height (copied from tfImageHeight)

// Helpers
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Stub XPutImage that will read from xImage->data based on the arguments.
// When sx + w > image width or sy + h > image height, the inner reads
// will go out of bounds, which ASan will catch as an OOB-read.
static void XPutImage(Display* dpy, Drawable d, GC gc, XImage* img,
                      int sx, int sy, int dx, int dy, unsigned int w, unsigned int h)
{
    (void)dpy; (void)d; (void)gc; (void)dx; (void)dy;
    volatile unsigned int sink = 0;
    // Simulate per-pixel read; assume 1 byte per pixel.
    for (unsigned int row = 0; row < h; row++) {
        for (unsigned int col = 0; col < w; col++) {
            size_t idx = (size_t)(sy + (int)row) * (size_t)img->width + (size_t)(sx + (int)col);
            // This will read out-of-bounds if sx + w > img->width or sy + h > img->height
            sink += img->data[idx];
        }
    }
    // Prevent optimizing away the loop
    if (sink == 0xdeadbeefU) fprintf(stderr, "sink: %u\n", sink);
}

// Vulnerable function logic reproduced from xtiff.c (Expose handling part)
static void EventProc(Widget widget, void* closure, XEvent* event)
{
    (void)closure;
    int sx, sy, dx, dy;
    unsigned int w, h;

    switch (event->type) {
        case Expose:
            // Vulnerable calculations: sx/sy include xOffset/yOffset,
            // while w/h are clamped only to image dimensions (iw/ih), not to remaining width/height.
            dx = event->xexpose.x;
            dy = event->xexpose.y;
            sx = xOffset + dx;
            sy = yOffset + dy;
            w = MIN((unsigned int)event->xexpose.width, (unsigned int)iw);
            h = MIN((unsigned int)event->xexpose.height, (unsigned int)ih);
            break;
        default:
            return;
    }

    if (appData.usePixmap) {
        // In real code this would use XCopyArea/Plane; we don't need it here.
        return;
    } else {
        // This call will read out-of-bounds from xImage->data if sx + w > img width or sy + h > img height.
        XPutImage(xDisplay, XtWindow(widget), xWinGc, xImage, sx, sy, dx, dy, w, h);
    }
}

int main(void)
{
    // Set up a small image buffer
    tfImageWidth = 16;
    tfImageHeight = 16;
    iw = (int)tfImageWidth;
    ih = (int)tfImageHeight;

    xImage = (XImage*)calloc(1, sizeof(XImage));
    if (!xImage) {
        perror("calloc xImage");
        return 1;
    }
    xImage->width = iw;
    xImage->height = ih;
    size_t buf_size = (size_t)iw * (size_t)ih; // 256 bytes
    xImage->data = (unsigned char*)malloc(buf_size);
    if (!xImage->data) {
        perror("malloc data");
        return 1;
    }
    memset(xImage->data, 0xAA, buf_size);

    // Choose non-zero xOffset to cause sx + w to exceed image width.
    // With sx = 8 and w = 16, sx + w = 24 > image width (16).
    xOffset = 8;  // yOffset can be zero for simplicity
    yOffset = 0;

    // Craft a fake Expose event whose width/height equal the full image size,
    // as done by ResizeProc in the original code path.
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = Expose;
    ev.xexpose.x = 0;
    ev.xexpose.y = 0;
    ev.xexpose.width = (int)tfImageWidth;   // full image width
    ev.xexpose.height = (int)tfImageHeight; // full image height

    // Trigger the vulnerable code path
    EventProc(/*widget*/NULL, /*closure*/NULL, &ev);

    // Cleanup (unlikely to be reached if ASan aborts on OOB)
    free(xImage->data);
    free(xImage);
    return 0;
}
