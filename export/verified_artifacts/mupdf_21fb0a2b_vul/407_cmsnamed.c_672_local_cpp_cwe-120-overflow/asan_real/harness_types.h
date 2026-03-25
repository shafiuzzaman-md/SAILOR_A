/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for cmsnamed.c:672 CWE-120 overflow */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Local type definitions (minimal)
typedef void* cmsContext;
typedef int cmsBool;
typedef uint32_t cmsUInt32Number;
typedef uint16_t cmsUInt16Number;

#ifndef TRUE
#define TRUE 1
#endif

// Minimal representation of a named color entry
typedef struct {
    char Name[64];
    char Prefix[32];
    char Suffix[32];
    cmsUInt16Number PCS[3];
    cmsUInt16Number *DeviceColorant; // source buffer
} _cmsNAMEDCOLOR;

// Minimal list holding colors and the ColorantCount
typedef struct {
    cmsUInt32Number ColorantCount;
    _cmsNAMEDCOLOR *List;
} cmsNAMEDCOLORLIST;

// Vulnerable function (neutralized). Keep the exact vulnerable statement text.
cmsBool cmsNamedColorInfo(cmsContext ContextID, const cmsNAMEDCOLORLIST* NamedColorList,
                          cmsUInt32Number nColor,
                          char* Name, char* Prefix, char* Suffix,
                          cmsUInt16Number* PCS, cmsUInt16Number* Colorant)
{
    if (Name) strcpy(Name, NamedColorList->List[nColor].Name);
    if (Prefix) strcpy(Prefix, NamedColorList->List[nColor].Prefix);
    if (Suffix) strcpy(Suffix, NamedColorList->List[nColor].Suffix);
    if (PCS)
        memmove(PCS, NamedColorList ->List[nColor].PCS, 3*sizeof(cmsUInt16Number));

    if (Colorant)
        memmove(Colorant, NamedColorList ->List[nColor].DeviceColorant,
                                sizeof(cmsUInt16Number) * NamedColorList ->ColorantCount);
    // Universal sink assertion after the vulnerable statement

    return TRUE;
}

// Entry function: direct pass-through with no guards
int harness_entry(cmsContext ctx, const cmsNAMEDCOLORLIST* ncl, cmsUInt32Number nColor,
                  char* Name, char* Prefix, char* Suffix,
