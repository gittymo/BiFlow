#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bglobal.h"
#include "bvalue.h"
#include "bnumber.h"

BScriptValue * BScriptCreateValue(BScriptValueType type)
{
    BScriptValue * value = (BScriptValue *) malloc(sizeof(BScriptValue));
    if (value) {
        value->id = BSCRIPT_DATA_STRUCT_ID;
        value->type = type;
        value->is_array = value->is_empty = false;
        value->array_length = value->array_max_size = 0;
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
    } else if (value->methods.freeFunction) {
        value->methods.freeFunction(value);
    }
    value->id = 0;
    value->is_empty = true;
    value->is_array = false;
    value->type = BScriptTypeUndefined;
    value->methods.freeFunction = NULL;
    value->methods.combineFunction = NULL;
    value->methods.asCharString = NULL;
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

BScriptValue * BScriptCombineValuesIntoArray(BScriptValue * left_value, BScriptValue * right_value)
{
    if (!left_value || left_value->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (!right_value || right_value->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (right_value->is_empty) return left_value;
    if (left_value->is_empty) return right_value;
    if (!left_value->is_array && !right_value->is_array) return NULL;

    BScriptValue * new_array_value = BScriptCreateValue(left_value->type);
    if (!new_array_value) return NULL;

    new_array_value->methods = left_value->methods;
    new_array_value->is_array = true;
    new_array_value->is_empty = false;

    size_t new_array_size = (left_value->is_array ? left_value->array_length : 1) + (right_value->is_array ? right_value->array_length : 1);
    new_array_value->data.array = (BScriptValue **) malloc(sizeof(BScriptValue *) * new_array_size);
    if (!new_array_value->data.array) {
        BScriptFreeValue(new_array_value);
        return NULL;
    } else {
        new_array_value->array_length = 0;
        new_array_value->array_max_size = new_array_size;
    }
    size_t w = 0;
    if (left_value->is_array) {
        for (size_t r = 0; r < left_value->array_length; r++) {
            new_array_value->data.array[w++] = left_value->data.array[r];
        }
    } else {
        new_array_value->data.array[w++] = left_value;
    }
    if (right_value->is_array) {
        for (size_t r = 0; r < right_value->array_length; r++) {
            new_array_value->data.array[w++] = right_value->data.array[r];
        }
    } else {
        new_array_value->data.array[w++] = right_value;
    }
    new_array_value->array_length = w;
    return new_array_value;
}

BScriptValue * BScriptCombineValues(BScriptValue * left_value, BScriptValue * right_value)
{
    if (!left_value || left_value->id != BSCRIPT_DATA_STRUCT_ID || left_value->type == BScriptTypeUndefined) return NULL;
    if (!right_value || right_value->id != BSCRIPT_DATA_STRUCT_ID || right_value->type == BScriptTypeUndefined) return NULL;
    if (left_value->is_empty) return right_value;
    else if (right_value->is_empty) return left_value;

    BScriptValue * combined_value = (BScriptValue *) malloc(sizeof(BScriptValue));
    if (!combined_value) return NULL;
    else {
        combined_value->id = BSCRIPT_DATA_STRUCT_ID;
        combined_value->is_array = false;
        combined_value->is_empty = false;
        combined_value->array_length = 0;
    }

    switch (left_value->type) {
        case BScriptTypeString : {
            char * right_value_string = NULL;
            size_t right_value_string_length = 0;
            switch (right_value->type) {
                case BScriptTypeString : {
                    right_value_string = (char *) malloc(right_value->data.string->length + 1);
                    if (right_value_string) {
                        right_value_string_length = sprintf(right_value_string, "%s", right_value->data.string->data);
                    }
                } break;
                case BScriptTypeNumber : {
                    right_value_string = BScriptGetNumberValueAsCharString(right_value, &right_value_string_length);
                } break;
                case BScriptTypeBoolean : {
                    right_value_string = (char *) malloc(6);
                    if (right_value_string) {
                        right_value_string_length = sprintf(right_value_string, "%s", right_value->data.boolean ? "true" : "false");
                    }
                } break;
                default: {
                    free(combined_value);
                    return NULL;
                }
            }
            if (!right_value_string) {
                free(combined_value);
                return NULL;
            }
            size_t combined_value_string_length = left_value->data.string->length + 1 + right_value_string_length;
            char * combined_value_string = (char *) malloc(combined_value_string_length);
            if (combined_value_string) {
                strncpy(combined_value_string, left_value->data.string->data, left_value->data.string->length);
                strncpy(combined_value_string + left_value->data.string->length, right_value->data.string->data, right_value->data.string->length);
                free(right_value_string);
            } else {
                free(combined_value);
                return NULL;
            }
            combined_value->data.string = BScriptCreateString(combined_value_string);
            combined_value->type = BScriptTypeString;
        }
    }

    return combined_value;
}