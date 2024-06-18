#include <stdio.h>

#include "bnumber.h"
#include "bglobal.h"
#include "bvalue.h"

BScriptValue * BScriptCreateNumberValue(double number, int decimal_places)
{
    BScriptValue * value = BScriptCreateValue(BScriptTypeNumber);
    if (value) {
        value->data.number = BScriptCreateNumber(number, decimal_places);
        value->methods.freeFunction = BScriptFreeNumberValue;
        value->methods.plusOperator = NULL; //TODO: Add number combination function.
        value->methods.valueAsString = BScriptGetNumberValueAsCharString;
    } 
    return value;
}

BScriptNumber * BScriptCreateNumber(double value, int decimal_places)
{
    BScriptNumber * num = (BScriptNumber *) malloc(sizeof(BScriptNumber));
    if (num) {
        num->value = value;
        num->id = BSCRIPT_DATA_STRUCT_ID;
        num->decimal_places = decimal_places >= 0 ? decimal_places : 3;
    }
    return num;
}

void BScriptFreeNumber(BScriptNumber * number)
{
    if (!number || number->id != BSCRIPT_DATA_STRUCT_ID) return;
    number->id = 0;
    number->value = 0;
    number->decimal_places = 0;
    free(number);
}

bool BScriptFreeNumberValue(BScriptValue * value)
{
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeNumber) return false;
    if (value->data.number) {
        BScriptFreeNumber(value->data.number);
        value->data.number = NULL;
    }
    return true;
}

char * BScriptGetNumberValueAsCharString(BScriptValue * number_value, size_t * string_length_ptr)
{
    if (!number_value || number_value->id != BSCRIPT_DATA_STRUCT_ID || number_value->type != BScriptTypeNumber || !string_length_ptr) return NULL;
    if (!number_value->data.number) return NULL;

    BScriptNumber * number = number_value->data.number;
    if (!number) return NULL;

    char format_string[12];
    int format_length;
    if (number->decimal_places == 0) {
        format_length = sprintf(format_string,"%%li");
    } else {
        format_length = sprintf(format_string,"%%.%df",number->decimal_places);
    }
    format_string[format_length] = 0;

    char * number_string = (char *) malloc(341);
    if (!number_string) return NULL;

    *string_length_ptr = sprintf(number_string, format_string, number->value);
    return number_string;
}