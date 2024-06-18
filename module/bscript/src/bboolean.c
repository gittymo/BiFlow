#include "bglobal.h"
#include "bboolean.h"
#include "bvalue.h"

BScriptValue * BScriptCreateBooleanValue(bool boolean_value)
{
    BScriptValue * value = BScriptCreateValue(BScriptTypeBoolean);
    if (value) {
        value->methods.freeFunction = BScriptFreeBooleanValue;
        value->methods.combineFunction = BScriptCombineBooleanValues;
        value->methods.asCharString = NULL; //TODO: Add combine boolean function.
        value->data.boolean = BScriptCreateBoolean(boolean_value);
    }
    return value;
}

BScriptBoolean * BScriptCreateBoolean(bool value)
{
    BScriptBoolean * boolean = (BScriptBoolean *) malloc(sizeof(BScriptBoolean));
    if (boolean) {
        boolean->id = BSCRIPT_DATA_STRUCT_ID;
        boolean->value = value;
    }
    return boolean;
}

void BScriptFreeBoolean(BScriptBoolean * boolean)
{
    if (!boolean || boolean->id != BSCRIPT_DATA_STRUCT_ID) return;
    boolean->id = 0;
    boolean->value = false;
    free(boolean);
}

bool BScriptFreeBooleanValue(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean) return false;
    if (value->data.boolean) {
        BScriptFreeBoolean(value->data.boolean);
        value->data.boolean = NULL;
    }
    return true;
}

BScriptValue * BScriptCombineBooleanValues(BScriptValue * left_value, BScriptValue * right_value)
{
    if (!left_value || left_value->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (!right_value || right_value->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (right_value->is_empty) return left_value;
    if (left_value->is_empty) return right_value;

    BScriptValue * new_value = NULL;

    if (!left_value->is_array && !right_value->is_array) {
        new_value = BScriptCreateValue(left_value->type);
        if (new_value) {
            new_value->methods = left_value->methods;
            new_value->data.boolean = BScriptCreateBoolean(left_value->data.boolean->value & right_value->data.boolean->value);
        }
    } else {
       new_value = BScriptCombineValuesIntoArray(left_value, right_value);
    }
    return new_value;
}