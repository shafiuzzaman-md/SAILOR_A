#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>

// Minimal local types to match harness definitions
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_stream { int dummy; } fz_stream;

// Prototype matching the harness implementation
fz_stream * fz_open_faxd(fz_context *ctx, fz_stream *chain,
    int k, int end_of_line, int encoded_byte_align,
    int columns, int rows, int end_of_block, int black_is_1);

int main() {
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_stream *chain = (fz_stream *)calloc(1, sizeof(fz_stream));

    int k, end_of_line, encoded_byte_align, columns, rows, end_of_block, black_is_1;
    { static const unsigned char k_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&k, k_data, (sizeof(k) < sizeof(k_data)) ? sizeof(k) : sizeof(k_data)); };
    { static const unsigned char end_of_line_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&end_of_line, end_of_line_data, (sizeof(end_of_line) < sizeof(end_of_line_data)) ? sizeof(end_of_line) : sizeof(end_of_line_data)); };
    { static const unsigned char encoded_byte_align_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&encoded_byte_align, encoded_byte_align_data, (sizeof(encoded_byte_align) < sizeof(encoded_byte_align_data)) ? sizeof(encoded_byte_align) : sizeof(encoded_byte_align_data)); };
    { static const unsigned char columns_data[] = {0xd9, 0xc8, 0xc0, 0x86}; memcpy(&columns, columns_data, (sizeof(columns) < sizeof(columns_data)) ? sizeof(columns) : sizeof(columns_data)); };
    { static const unsigned char rows_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&rows, rows_data, (sizeof(rows) < sizeof(rows_data)) ? sizeof(rows) : sizeof(rows_data)); };
    { static const unsigned char end_of_block_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&end_of_block, end_of_block_data, (sizeof(end_of_block) < sizeof(end_of_block_data)) ? sizeof(end_of_block) : sizeof(end_of_block_data)); };
    { static const unsigned char black_is_1_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&black_is_1, black_is_1_data, (sizeof(black_is_1) < sizeof(black_is_1_data)) ? sizeof(black_is_1) : sizeof(black_is_1_data)); };

    // Direct call to the entry/vulnerable function
    fz_open_faxd(ctx, chain, k, end_of_line, encoded_byte_align, columns, rows, end_of_block, black_is_1);
    return 0;
}
