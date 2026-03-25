#include <stdio.h>
#include <stdlib.h>

/* Rename the addtiffo tool's main() so we can include it and call it directly */
#define main addtiffo_main
#include "contrib/addtiffo/addtiffo.c"
#undef main

int main(void)
{
    /* Craft argv to only contain the -subifd option and no filename */
    char prog[] = "repro";
    char opt[] = "-subifd";
    char *argv[] = { prog, opt, NULL };
    int argc = 2; /* program name + one option */

    /* This call will trigger the NULL pointer dereference inside addtiffo_main */
    return addtiffo_main(argc, argv);
}
