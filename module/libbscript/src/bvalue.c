#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bglobal.h"
#include "bvalue.h"
#include "bnumber.h"

void BScriptValueInitMethods(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID) return;
    value->methods.typeAsString = NULL;
    value->methods.free = NULL;

    value->methods.plusOperator = NULL;
    value->methods.minusOperator = NULL;
    value->methods.multiplyOperator = NULL;
    value->methods.divideOperator = NULL;
    value->methods.modulusOperator = NULL;

    value->methods.orOperator = NULL;
    value->methods.andOperator = NULL;
    value->methods.notOperator = NULL;
    value->methods.xorOperator = NULL;

    value->methods.equalOperator = NULL;
    value->methods.notEqualOperator = NULL;
    value->methods.greaterThanOperator = NULL;
    value->methods.lessThanOperator = NULL;

    value->methods.valueAsString = NULL;
    value->methods.valueAsNumber = NULL;
    value->methods.valueAsBoolean = NULL;

    value->methods.addToArray = NULL;
    value->methods.createCombinedArray = NULL;
}

BScriptValue * BScriptCreateValue(BScriptValueType type)
{
    BScriptValue * value = (BScriptValue *) malloc(sizeof(BScriptValue));
    if (value) {
        value->id = BSCRIPT_DATA_STRUCT_ID;
        value->type = type;
        value->is_array = value->is_empty = false;
        value->array_length = value->array_max_size = 0;
        BScriptValueInitMethods(value);
    }
    return value;
}

bool BScriptFreeValue(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (value->is_array) {
        for (size_t r = 0; r < value->array_length; r++) {
            if (value->data.array[r]) {
                BScriptFreeValue(value->data.array[r]);
                value->data.array[r] = NULL;
            }
            free(value->data.array);
            value->data.array = NULL;
        }
        value->array_length = value->array_max_size = 0;
    } else if (value->methods.free) {
        value->methods.free(value);
    }
    value->id = 0;
    value->is_empty = true;
    value->is_array = false;
    value->type = BScriptTypeUndefined;

    BScriptValueInitMethods(value);

    free(value);
    return true;
}

bool BScriptResizeValueArray(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (!value->is_array) return false;
    if (!value->data.array) return false;
    if (value->array_max_size - value->array_length < 16) {
        const size_t new_max_size = value->array_max_size + 16;
        BScriptValue ** new_value_array = (BScriptValue **) realloc(value->data.array, sizeof(BScriptValue *) * new_max_size);
        if (!new_value_array) return false;
        else value->data.array = new_value_array;
    }
    return true;
}

bool BScriptValueDefaultChecks(BScriptValue * val1, BScriptValue * val2)
{
    if (!val1 || val1->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (!val2 || val2->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (val1->is_empty || !val2->is_empty) return false;
    return true;
}

bool BScriptValueEqualsOperator(BScriptValue * val1, BScriptValue * val2)
{
    if (!BScriptValueDefaultChecks(val1, val2)) return false;
    if (!val1->is_array && !val2->is_array) return val1->methods.equalOperator(val1, val2);
    if (val1->is_array) {
        if (val2->is_array) {
            if (val1->array_length != val2->array_length) return false;
            else {
                bool equal = true;
                for (size_t i = 0; i < val1->array_length && equal; i++) {
                    equal = val1->data.array[i]->methods.equalOperator(val1->data.array[i], val2->data.array[i]);
                }
                return equal;
            }
        } else if (val1->array_length == 1) {
            return val1->methods.equalOperator(val1->data.array[0], val2);
        } else return false;
    } else if (val2->is_array) {
        if (val2->array_length != 1) return false;
        return val1->methods.equalOperator(val1, val2->data.array[0]);
    }
    return false;
}