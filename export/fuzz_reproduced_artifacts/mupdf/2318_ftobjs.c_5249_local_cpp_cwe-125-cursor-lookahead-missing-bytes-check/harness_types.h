/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal local typedefs to model FreeType structures used on the path

typedef struct FT_Module_Class_ {
    void* (*get_interface)(void* module, const char* service_id);
} FT_Module_Class;

typedef struct FT_LibraryRec_ FT_LibraryRec;

typedef struct FT_ModuleRec_ {
    FT_Module_Class* clazz;
    struct FT_LibraryRec_* library;
} FT_ModuleRec, *FT_Module;

struct FT_LibraryRec_ {
    FT_Module* modules;      // array of FT_Module
    unsigned int num_modules;
};

typedef struct FT_LibraryRec_* FT_Library;

