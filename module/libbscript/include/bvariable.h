#ifndef COM_PLUS_MEVANSPN_BSCRIPT_VARIABLE
#define COM_PLUS_MEVANSPN_BSCRIPT_VARIABLE

#include "bstring.h"
#include "bvalue.h"

typedef struct bscript_variable {
    int id;
    BScriptString * name;
    BScriptValue * value;
} BScriptVariable;

BScriptVariable * BScriptCreateVariable(char * name, char * value, BScriptValueType variable_type);
bool BScriptFreeVariable(BScriptVariable * var);

#endif