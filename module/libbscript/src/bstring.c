#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bglobal.h"
#include "bstring.h"
#include "bvalue.h"

char * BScriptStringAsCharString(BScriptValue * value);
double BScriptStringAsNumber(BScriptValue * value);
bool BScriptStringAsBoolean(BScriptValue * value);

BScriptValue * BScriptCreateStringValue(char * string)
{
    BScriptValue * value = BScriptCreateValue(BScriptTypeString);
    if (value) {
        value->data.string = BScriptCreateString(string);
        value->methods.free = BScriptFreeStringValue;
        value->methods.plusOperator = NULL; //TODO: Add string combination function.
        value->methods.valueAsString = BScriptStringAsCharString;
        value->methods.valueAsNumber = BScriptStringAsNumber;
        value->methods.valueAsBoolean = BScriptStringAsBoolean;
    }
    return value;
}

BScriptString * BScriptCreateString(char * string)
{
    BScriptString * str = (BScriptString *) malloc(sizeof(BScriptString));
    if (str) {
        str->id = BSCRIPT_DATA_STRUCT_ID;
        if (string) {
            size_t l = 0;
            while (string[l]) l++;
            str->data = (char *) malloc(l + 1);
            if (str->data) {
                strcpy(str->data, string);
                str->length = l;
            } else {
                free(str);
                return NULL;
            }
        } else {
            str->data = string;
            str->length = 0;
        }
    }
    str->hash = BScriptGetHash(string);
    return str;
}

bool BScriptFreeString(BScriptString * string)
{
    if (!string || string->id != BSCRIPT_DATA_STRUCT_ID) return false;
    string->id = 0;
    if (string->data) {
        free(string->data);
        string->data = NULL;
    }
    string->hash = 0;
    string->length = 0;
    free(string);
    return true;
}

bool BScriptFreeStringValue(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeString) return false;
    if (value->data.string) {
        BScriptFreeString(value->data.string);
        value->data.string = NULL;
    }
    return true;
}

char * BScriptStringAsCharString(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeString || value->is_empty) return NULL;
    int char_string_length = 0;
    if (!value->is_array && value->data.string->length > 0) char_string_length = value->data.string->length + 1;
    else {
        for (size_t s = 0; s < value->array_length; s++) {
            BScriptString * bstr = value->data.array[s];
            if (bstr->length > 0) char_string_length += bstr->length + 1;
        }
    }
    int w = 0;
    char * char_string = (char *) malloc(char_string_length + 1);
    if (!value->is_array && value->data.string->length > 0) {
        w = sprintf(char_string, "%s", value->data.string->data);
    } else {
        for (size_t s = 0; s < value->array_length; s++) {
            BScriptString * bstr = value->data.array[s];
            if (bstr->length > 0) {
                int chars_written = sprintf(char_string + w, "%s%c", value->data.array[s]->data.string->data, s < value->array_length - 1 ? ',' : 0);
                w += chars_written;
            }
        }
    }
    char_string[w] = 0;
    return char_string;
}

double BScriptStringAsNumber(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeString || value->is_empty) return 0;
    if (!value->is_array) {
        if (BScriptTokenIsNumericConstant(value->data.string->data)) return atof(value->data.string->data);
    } else {
        return 0;
    }
}

bool BScriptStringAsBoolean(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeString || value->is_empty) return false;
    if (!value->is_array) {
        if (BScriptTokenIsBooleanConstant(value->data.string->data)) return strcmp(value->data.string->data,"true") == 0 ? true : false;
        else return false;
    } else {
        return value->array_length > 0;
    }
}