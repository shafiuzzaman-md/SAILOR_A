/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>

// Minimal types to compile js_remove and reach the sink
typedef struct js_Value {
    int tag;
    void *ptr;
} js_Value;

typedef struct js_State {
    int bot;
    int top;
    js_Value *stack;
} js_State;

// Macros matching usage in jsrun.c snippet
#define BOT (J->bot)
#define TOP (J->top)
