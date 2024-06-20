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
BScriptBoolean * BScriptCreateBoolean(bool value);
void BScriptFreeBoolean(BScriptBoolean * boolean);

bool BScriptBooleanValueAsBoolean(BScriptValue *value);
double BScriptBooleanValueAsNumber(BScriptValue *value);
char * BScriptBooleanValueAsString(BScriptValue *value);
bool BScriptBooleanAddValueToArray(BScriptValue * array_value, BScriptValue * value_to_add);
BScriptValue * BScriptBooleanValuePlusOperation(BScriptValue * value1, BScriptValue * value2);

#endif