/* driver.c */
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

FT_Error FT_Select_Charmap( FT_Face face, FT_Encoding encoding );

int main(void) {
    /* 1) Concrete allocations */
    FT_Face face = (FT_Face)calloc(1, sizeof(struct FT_FaceRec_));

    const int CAP = 2;
    FT_CharMap *arr = (FT_CharMap*)calloc(CAP, sizeof(FT_CharMap));

    FT_CharMap map0 = (FT_CharMap)calloc(1, sizeof(struct FT_CharMapRec_));
    FT_CharMap map1 = (FT_CharMap)calloc(1, sizeof(struct FT_CharMapRec_));

    arr[0] = map0;
    arr[1] = map1;

    face->charmaps = arr;

    /* 2) Symbolic fields via temporaries, then assign (avoid subobject symbolic) */
    unsigned int u32;
    unsigned short u16;

    memset(&u32, 0, sizeof(u32)); /* replay: no ktest data for "map0_encoding" */;
    map0->encoding = u32;
    memset(&u16, 0, sizeof(u16)); /* replay: no ktest data for "map0_pid" */;
    map0->platform_id = u16;
    memset(&u16, 0, sizeof(u16)); /* replay: no ktest data for "map0_eid" */;
    map0->encoding_id = u16;

    { static const unsigned char map1_encoding_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&u32, map1_encoding_data, (sizeof(u32) < sizeof(map1_encoding_data)) ? sizeof(u32) : sizeof(map1_encoding_data)); };
    map1->encoding = u32;
    memset(&u16, 0, sizeof(u16)); /* replay: no ktest data for "map1_pid" */;
    map1->platform_id = u16;
    { static const unsigned char map1_eid_data[] = {0x00, 0x00}; memcpy(&u16, map1_eid_data, (sizeof(u16) < sizeof(map1_eid_data)) ? sizeof(u16) : sizeof(map1_eid_data)); };
    map1->encoding_id = u16;

    /* 3) Symbolic num_charmaps via temp, constrain to [0, CAP] to allow cur-- edge */
    int ncm;
    { static const unsigned char num_charmaps_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&ncm, num_charmaps_data, (sizeof(ncm) < sizeof(num_charmaps_data)) ? sizeof(ncm) : sizeof(num_charmaps_data)); };
    /* klee_assume removed */
    /* klee_assume removed */
    face->num_charmaps = ncm;

    /* 4) Call entry */
    FT_Encoding enc;
    { static const unsigned char encoding_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&enc, encoding_data, (sizeof(enc) < sizeof(encoding_data)) ? sizeof(enc) : sizeof(encoding_data)); };
    (void)FT_Select_Charmap(face, enc);

    return 0;
}
