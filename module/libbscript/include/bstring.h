#ifndef COM_PLUS_MEVANSPN_BSCRIPT_STRING
#define COM_PLUS_MEVANSPN_BSCRIPT_STRING

#include <stdbool.h>

typedef struct bscript_string {
    int id;
    char * data;
    int length;
    int hash;
} BScriptString;

BScriptString * BScriptCreateString(char * string);
bool BScriptFreeString(BScriptString * string);
struct bscript_value * BScriptCreateStringValue(char * string);
bool BScriptFreeStringValue(struct bscript_value * value);

#endif