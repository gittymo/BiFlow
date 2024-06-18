#include <stdlib.h>
#include <string.h>

#include "bglobal.h"
#include "bstring.h"
#include "bvalue.h"

BScriptValue * BScriptCreateStringValue(char * string)
{
    BScriptValue * value = BScriptCreateValue(BScriptTypeString);
    if (value) {
        value->data.string = BScriptCreateString(string);
        value->methods.freeFunction = BScriptFreeStringValue;
        value->methods.combineFunction = NULL; //TODO: Add string combination function.
        value->methods.asCharString = NULL; //TODO: Add as char string function.
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