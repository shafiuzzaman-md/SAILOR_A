#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/*
 This standalone program reproduces the out-of-bounds read from
 contrib/libtests/timepng.c:552 by emulating the same logic:
 - Read a filename from stdin using fgets
 - If the line does not end with a newline (e.g., last line without \n),
   print an error using filename+len-32, which underflows for short lines.
*/
int main(void)
{
    /* Prepare stdin to contain a short line without a trailing newline. */
    int fds[2];
    if (pipe(fds) != 0) {
        perror("pipe");
        return 1;
    }

    const char *short_line_no_newline = "shortname"; /* len = 9 (< 32) */
    ssize_t w = write(fds[1], short_line_no_newline, (unsigned)strlen(short_line_no_newline));
    (void)w; /* ignore write result in reproducer */
    close(fds[1]);

    /* Redirect the read end of the pipe to stdin (fd 0). */
    if (dup2(fds[0], 0) == -1) {
        perror("dup2");
        close(fds[0]);
        return 1;
    }
    close(fds[0]);

    /* Reproduce the vulnerable stdin-processing block from timepng.c */
    char filename[FILENAME_MAX+1];

    while (fgets(filename, FILENAME_MAX+1, stdin))
    {
        size_t len = strlen(filename);

        if (filename[len-1] == '\n')
        {
            filename[len-1] = 0;
            /* In the original code this would try to add the file, but it's
               irrelevant for triggering the bug. */
        }
        else
        {
            /* BUG: If len < 32 (true here: 9), this underflows the pointer
               before the start of the filename buffer, causing an OOB read
               when fprintf processes %s. */
            fprintf(stderr, "timepng: file name too long: ...%s\n", filename + len - 32);
            break;
        }
    }

    return 0;
}
