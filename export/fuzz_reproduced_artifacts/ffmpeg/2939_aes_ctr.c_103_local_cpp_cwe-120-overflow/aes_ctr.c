#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif
#ifndef AES_CTR_IV_SIZE
#define AES_CTR_IV_SIZE 16
#endif

// Minimal struct with only fields needed on path
struct AVAESCTR {
    uint8_t *counter;                       // pointer-typed to reflect potential bug scenario
    uint8_t encrypted_counter[AES_BLOCK_SIZE];
    int block_offset;
};

static void av_aes_ctr_increment_be64(uint8_t* counter)
{
    uint8_t* cur_pos;
    for (cur_pos = counter + 7; cur_pos >= counter; cur_pos--) {
        (*cur_pos)++;
        if (*cur_pos != 0) {
            break;
        }
    }
}

void av_aes_ctr_increment_iv(struct AVAESCTR *a)
{
    av_aes_ctr_increment_be64(a->counter);
    // Vulnerable statement: MUST be verbatim per summary
    memset(a->counter + AES_CTR_IV_SIZE, 0, sizeof(a->counter) - AES_CTR_IV_SIZE);
    // Universal sink assertion placed immediately AFTER the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
    a->block_offset = 0;
}

// Entry must be a direct pass-through with no guards
int entry_func(struct AVAESCTR *a)
{
    av_aes_ctr_increment_iv(a);
    return 0;
}
