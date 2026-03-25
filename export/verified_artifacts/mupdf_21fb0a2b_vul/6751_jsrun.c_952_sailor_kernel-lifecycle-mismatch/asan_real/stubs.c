#include "harness_types.h"
// klee removed

int jsR_delproperty(js_State *J, js_Object *obj, const char *name)
{
    // Overapproximate behavior: do not dereference pointers here.
    int ret;
    memset(&ret, sizeof(ret), "jsR_delproperty_ret") /* stub */;;
    return ret;
}
