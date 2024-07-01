#ifndef COM_PLUS_MEVANSPN_BSCRIPT_BOOLEAN
#define COM_PLUS_MEVANSPN_BSCRIPT_BOOLEAN

#include <stdbool.h>
#include "bglobal.h"

typedef struct bscript_boolean {
    int id;
    bool value;
} BScriptBoolean;

struct bscript_value * BScriptCreateBooleanValue(bool boolean_value);
bool BScriptFreeBooleanValue(struct bscript_value * value);

#endif