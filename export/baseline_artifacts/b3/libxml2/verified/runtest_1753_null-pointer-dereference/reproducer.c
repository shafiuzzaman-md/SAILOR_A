#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Stub out libxml2 allocation wrappers used by runtest.c */
void *xmlMalloc(size_t size) {
    (void)size;
    /* Simulate allocation failure to trigger the bug path */
    return NULL;
}

void xmlFree(void *ptr) {
    (void)ptr;
}

/* Minimal reproduction of the vulnerable code sequence from htmlTokenizerTest */
static void trigger_htmlTokenizerTest_like_bug(const char *filename) {
    FILE *input = fopen(filename, "rb");
    if (!input) {
        perror("fopen");
        exit(1);
    }

    unsigned testNum, dataState, size;
    char startTag[31];

    /* Parse the header line exactly like the original code */
    if (fscanf(input, "%u %30s %u %u%*1[\n]", &testNum, startTag, &dataState, &size) >= 4) {
        char *data;

        /* This call returns NULL due to our stub above */
        data = (char *)xmlMalloc(size + 1);

        /* Vulnerable call: fread writes into a NULL pointer when allocation fails */
        /* This should crash with a NULL pointer dereference under ASan */
        if (fread(data, 1, size, input) != size) {
            /* If it didn't crash (unexpected), bail out with an error */
            fprintf(stderr, "Unexpected: fread did not read expected size (read error or short read)\n");
            fclose(input);
            exit(2);
        }

        /* Not reached; included for completeness */
        xmlFree(data);
    } else {
        fprintf(stderr, "Input format error\n");
        fclose(input);
        exit(3);
    }

    fclose(input);
}

int main(void) {
    /* Create a temporary input file that matches the expected format: 
       "<testNum> <startTag> <dataState> <size>\n" followed by <size> bytes of data. */
    char tmpl[] = "/tmp/libxml2_htmltok_repro.XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        perror("mkstemp");
        return 1;
    }

    FILE *f = fdopen(fd, "wb");
    if (!f) {
        perror("fdopen");
        close(fd);
        unlink(tmpl);
        return 1;
    }

    unsigned size = 16; /* any non-zero size triggers the bug when xmlMalloc returns NULL */
    const char *header = "1 start 0 16\n"; /* testNum=1, startTag=\"start\", dataState=0, size=16 */
    const char payload[] = "1234567890ABCDEF"; /* 16 bytes */

    if (fwrite(header, 1, strlen(header), f) != strlen(header)) {
        perror("fwrite header");
        fclose(f);
        unlink(tmpl);
        return 1;
    }
    if (fwrite(payload, 1, size, f) != size) {
        perror("fwrite payload");
        fclose(f);
        unlink(tmpl);
        return 1;
    }

    fflush(f);
    fclose(f);

    /* Trigger the vulnerable sequence */
    trigger_htmlTokenizerTest_like_bug(tmpl);

    /* Clean up (likely not reached due to crash) */
    unlink(tmpl);
    return 0;
}