#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

int gumbo_tag_from_original_text(GumboStringPiece* text);

int main() {
    // Concrete 1-byte buffer to trigger OOB read on data[1]
    char *buf = (char*)malloc(1);
    if (!buf) return 0;
    // Make content symbolic to let KLEE explore; size is concrete per instructions
    { static const unsigned char buf1_data[] = {0x00}; memcpy(buf, buf1_data, (1 < sizeof(buf1_data)) ? 1 : sizeof(buf1_data)); };

    GumboStringPiece text;
    text.data = buf;
    text.length = 1;  // less than 2 triggers lookahead OOB on data[1]

    // Call entry (simple pass-through to vulnerable function)
    gumbo_tag_from_original_text(&text);
    return 0;
}
