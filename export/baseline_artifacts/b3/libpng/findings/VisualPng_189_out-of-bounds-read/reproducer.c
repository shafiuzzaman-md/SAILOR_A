#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal Windows-like type and constant stubs */
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HDC;
typedef void* HMENU;
typedef unsigned int UINT;
typedef unsigned long WPARAM;
typedef unsigned long LPARAM;
typedef long LRESULT;
typedef int BOOL;
typedef unsigned char BYTE;
typedef char TCHAR;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define WM_CREATE 0x0001
#define WM_SIZE   0x0005

#ifndef MAX_PATH
#define MAX_PATH 16  /* small on purpose to make the issue surface quickly */
#endif

/* Dummy structs to satisfy the original code context */
typedef struct { int dummy; } PAINTSTRUCT;
typedef struct { int dummy; } BITMAPFILEHEADER;
typedef struct { int dummy; } BITMAPINFOHEADER;
typedef struct { BYTE red, green, blue; } png_color;

typedef struct {
    HINSTANCE hInstance;
    /* other fields not needed */
} CREATESTRUCT_, *LPCREATESTRUCT;

/* Global that simulates the command-line filename buffer from VisualPng.c */
static TCHAR szCmdFileName[MAX_PATH];

/* Stubs for functions used by WndProc */
static void PngFileInitialize(HWND hwnd) { (void)hwnd; }
static void BuildPngList(TCHAR *path, TCHAR **pList, int *pCount, int *pIndex) {
    (void)path; (void)pList; (void)pCount; (void)pIndex;
}
static BOOL LoadImageFile(HWND hwnd, TCHAR *path,
                          BYTE **ppImg, int *pcx, int *pcy, int *pchan, png_color *pbkg) {
    (void)hwnd; (void)path; (void)ppImg; (void)pcx; (void)pcy; (void)pchan; (void)pbkg;
    return FALSE; /* value irrelevant; bug occurs before this is checked */
}
static void InvalidateRect(HWND hwnd, void *rect, BOOL erase) { (void)hwnd; (void)rect; (void)erase; }
static void DisplayImage(HWND hwnd, BYTE **pDib, BYTE **pDiData, int cxWin, int cyWin,
                         BYTE *pImg, int cxImg, int cyImg, int cChan, BOOL bStretched) {
    (void)hwnd; (void)pDib; (void)pDiData; (void)cxWin; (void)cyWin;
    (void)pImg; (void)cxImg; (void)cyImg; (void)cChan; (void)bStretched;
}

/* Reimplementation of the vulnerable WndProc logic focusing on the buggy path */
static LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    (void)wParam;

    static HINSTANCE          hInstance;
    static HDC                hdc;
    static PAINTSTRUCT        ps;
    static HMENU              hMenu;

    static BITMAPFILEHEADER  *pbmfh;
    static BITMAPINFOHEADER  *pbmih;
    static BYTE              *pbImage;
    static int                cxWinSize, cyWinSize;
    static int                cxImgSize, cyImgSize;
    static int                cImgChannels;
    static png_color          bkgColor = {127, 127, 127};

    static BOOL               bStretched = TRUE;

    static BYTE              *pDib = NULL;
    static BYTE              *pDiData = NULL;

    static TCHAR              szImgPathName [MAX_PATH];
    static TCHAR              szTitleName [MAX_PATH];

    static TCHAR             *pPngFileList = NULL;
    static int                iPngFileCount;
    static int                iPngFileIndex;

    BOOL                      bOk;

    switch (message)
    {
    case WM_CREATE:
        hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
        PngFileInitialize(hwnd);

        strcpy(szImgPathName, "");

        /* in case we process file given on command-line */
        if (szCmdFileName[0] != '\0')
        {
            /* Vulnerable: szCmdFileName may not be NUL-terminated; strcpy reads past end */
            strcpy(szImgPathName, szCmdFileName);

            BuildPngList(szImgPathName, &pPngFileList, &iPngFileCount, &iPngFileIndex);

            if (!LoadImageFile(hwnd, szImgPathName,
                               &pbImage, &cxImgSize, &cyImgSize, &cImgChannels, &bkgColor))
                return 0;

            InvalidateRect(hwnd, NULL, TRUE);
            DisplayImage(hwnd, &pDib, &pDiData, cxWinSize, cyWinSize,
                         pbImage, cxImgSize, cyImgSize, cImgChannels, bStretched);
        }
        return 0;

    case WM_SIZE:
        cxWinSize = (int)(lParam & 0xFFFF);
        cyWinSize = (int)((lParam >> 16) & 0xFFFF);
        InvalidateRect(hwnd, NULL, TRUE);
        DisplayImage(hwnd, &pDib, &pDiData, cxWinSize, cyWinSize,
                     pbImage, cxImgSize, cyImgSize, cImgChannels, bStretched);
        return 0;
    }

    return 0;
}

int main(void)
{
    /* Simulate the earlier strncpy behavior that fails to NUL-terminate when count == length
       by filling the entire szCmdFileName buffer with non-zero bytes (no NUL anywhere). */
    for (size_t i = 0; i < sizeof(szCmdFileName); i++) {
        szCmdFileName[i] = 'A';
    }

    /* Confirm the precondition used by the vulnerable code */
    if (szCmdFileName[0] == '\0') {
        fprintf(stderr, "Unexpected: szCmdFileName[0] == 0\n");
        return 1;
    }

    CREATESTRUCT_ cs;
    cs.hInstance = (HINSTANCE)0x1;

    /* Trigger WM_CREATE to hit the vulnerable strcpy */
    (void)WndProc((HWND)0x1, WM_CREATE, 0, (LPARAM)&cs);

    /* If ASan did not already abort, print something (but normally it will crash above). */
    puts("If you see this, ASan did not catch the OOB read (unexpected).\n");
    return 0;
}
