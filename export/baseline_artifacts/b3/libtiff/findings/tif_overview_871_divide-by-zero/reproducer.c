#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
   Minimal, self-contained stand-in for the vulnerable function from
   contrib/addtiffo/tif_overview.c. We replicate the core logic that
   triggers the divide-by-zero when an overview factor of 0 is supplied.
*/

typedef struct TIFF TIFF; /* Opaque placeholder; we don't use it here. */

/*
   Stand-in for contrib/addtiffo/tif_overview.c:TIFFBuildOverviews()
   Only the relevant portion is reproduced to trigger the bug.
*/
int TIFFBuildOverviews(TIFF *hTIFF, int nOverviews, const uint32_t *panOvList)
{
    /* Typical image sizes; actual values don't matter for the crash. */
    uint32_t nXSize = 1024;
    uint32_t nYSize = 768;

    /* Loop over requested overview factors. */
    for (int i = 0; i < nOverviews; i++)
    {
        /*
           Vulnerable computation from the original code:
             nOXSize = (nXSize + panOvList[i] - 1) / panOvList[i];
             nOYSize = (nYSize + panOvList[i] - 1) / panOvList[i];
           If panOvList[i] == 0, this divides by zero.
        */
        uint32_t nOXSize = (nXSize + panOvList[i] - 1) / panOvList[i];
        uint32_t nOYSize = (nYSize + panOvList[i] - 1) / panOvList[i];

        /* Prevent unused variable warnings */
        (void)nOXSize;
        (void)nOYSize;
    }

    return 0;
}

int main(void)
{
    /* Craft an overview list with a zero factor to trigger the bug. */
    uint32_t panOvList[1];
    panOvList[0] = 0; /* This is the invalid overview factor causing divide-by-zero */

    TIFF *tif = NULL; /* Not used by our minimal stand-in */

    /* Call the vulnerable function: this should raise SIGFPE (divide-by-zero). */
    (void)TIFFBuildOverviews(tif, 1, panOvList);

    /* If we got here (which we shouldn't), print something. */
    puts("Unexpectedly survived divide-by-zero");
    return 0;
}
