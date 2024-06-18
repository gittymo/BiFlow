#include <stdio.h>

#include "bglobal.h"
#include "bboolean.h"
#include "bvalue.h"

BScriptValue * BScriptCreateBooleanValue(bool boolean_value)
{
    BScriptValue * value = BScriptCreateValue(BScriptTypeBoolean);
    if (value) {
        value->methods.freeFunction = BScriptFreeBooleanValue;
        value->methods.plusOperator = BScriptBooleanValuePlusOperation;
        value->methods.valueAsString = BScriptBooleanValueAsString;
        value->methods.valueAsNumber = BScriptBooleanValueAsNumber;
        value->methods.valueAsBoolean = BScriptBooleanValueAsBoolean;
        value->methods.addToArray = BScriptBooleanAddValueToArray;
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

bool BScriptBooleanValueAsBoolean(BScriptValue *value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean) return false;
    return !value->is_array ? value->data.boolean->value : value->array_length > 0;
}

double BScriptBooleanValueAsNumber(BScriptValue *value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean) return 0.0;
    return !value->is_array? value->data.boolean->value ? 1.0 : 0.0 : value->array_length > 0;
}

char * BScriptBooleanValueAsString(BScriptValue *value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean) return NULL;
    char * return_string = NULL;

    if (!value->is_array) {
        char * boolean_string_value = (char *) malloc(6);
        if (boolean_string_value) {
            size_t string_length = sprintf(boolean_string_value, "%s", value->data.boolean->value ? "true" : "false");
            boolean_string_value[string_length] = 0;
        }
        return_string = boolean_string_value;
    } else {
        if (value->array_length < 1) return NULL;
        return_string = (char *) malloc(6 * value->array_length);
        if (return_string) {
            size_t w = 0;
            for (size_t r = 0; r < value->array_length; r++) {
                bool boolean_value = value->data.array[r]->methods.valueAsBoolean(value->data.array[r]);
                size_t string_length = sprintf(return_string + w, "%s", value->data.boolean->value ? "true" : "false");
                w += string_length;
                return_string[w++] = r < value->array_length - 1 ? ',' : 0;
            }
        }
    }
    return return_string;
}

bool BScriptBooleanAddValueToArray(BScriptValue * array_value, BScriptValue * value_to_add)
{
    if (!array_value || array_value->id != BSCRIPT_DATA_STRUCT_ID || !array_value->is_array || array_value->type != BScriptTypeBoolean) return false;
    if (!value_to_add || value_to_add->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (value_to_add->is_array) {
        if (array_value->array_max_size - array_value->array_length < value_to_add->array_length) {
            while (array_value->array_max_size - array_value->array_length < value_to_add->array_length) BScriptResizeValueArray(array_value);
            for (size_t r = 0; r < value_to_add->array_length; r++) {
                BScriptValue * boolean_value = BScriptCreateBooleanValue(value_to_add->methods.valueAsBoolean(value_to_add->data.array[r]));
                array_value->data.array[array_value->array_length++] = boolean_value;
            }
        }
    } else {
        if (array_value->array_length == array_value->array_max_size) BScriptResizeValueArray(array_value);
        array_value->data.array[array_value->array_length++] = BScriptCreateBooleanValue(value_to_add->methods.valueAsBoolean(value_to_add));
    }
}

BScriptValue * BScriptBooleanValuePlusOperation(BScriptValue * value1, BScriptValue * value2)
{
    if (!value1 || !value2) return NULL;
    if (value1->id != BSCRIPT_DATA_STRUCT_ID || value2->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (!value1->is_array && !value2->is_array) {
        bool result = value1->methods.valueAsBoolean(value1) & value2->methods.valueAsBoolean(value2);
        return BScriptCreateBooleanValue(result);
    } else if (value1->is_array) {
        value1->methods.addToArray(value1, value2);
        return value1;
    } else if (value2->is_array) {
        return BScriptCombineValuesIntoArray(value1, value2);
    }
}