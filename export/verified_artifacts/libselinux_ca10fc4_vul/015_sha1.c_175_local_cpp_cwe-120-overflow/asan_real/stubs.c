#include "harness_types.h"
// klee removed
#include <stdint.h>

void TransformFunction(uint32_t state[5], const uint8_t buffer[64]) {
    // Over-approximate behavior; do nothing functional, keep symbolic knob if needed
    int tf_sym;
    memset(&tf_sym, sizeof(tf_sym), "TransformFunction_sym") /* stub */;;
    (void)state; (void)buffer; (void)tf_sym;
}
