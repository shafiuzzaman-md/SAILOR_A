#include <stdint.h>
extern void TIFFSwabLong(uint32_t *lp);
int swab_entry(uint32_t *lp) {
    TIFFSwabLong(lp);
    return 0;
}
