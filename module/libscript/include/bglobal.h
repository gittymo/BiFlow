#ifndef COM_PLUS_MEVANSPN_BSCRIPT_GLOBAL
#define COM_PLUS_MEVANSPN_BSCRIPT_GLOBAL

#include <stdbool.h>
#include <stdlib.h>

#define BSCRIPT_DATA_STRUCT_ID (('B' << 24) + ('S' << 16) + ('c' << 8) + 'r')
#define BSCRIPT_MAX_NAME_LENGTH 32

const char * RESERVED_WORDS[] = {
    "var", "if", "else", "true", "false", "empty", "string", "number", "boolean", "and", "or", "=", "==", "!=", "+", "-", "/", "*", "%", NULL
};

const char * RESERVED_OPERATOR_CHARS = "=+-/*%()[]!";

struct bscript_value;

int BScriptGetHash(char * string);

bool BScriptCharIsReservedOperatorChar(char c);
bool BScriptTokenIsReservedKeyword(char * token_string);
bool BScriptTokenIsNumericConstant(char * token_string);
bool BScriptTokenIsBooleanConstant(char * token_string);
bool BScriptTokenIsStringConstant(char * token_string);
bool BScriptTokenIsValidVariableName(char * token_string);

#endif