#ifndef COM_PLUS_MEVANSPN_BSCRIPT
#define COM_PLUS_MEVANSPN_BSCRIPT

#include <stdlib.h>
#include "bvalue.h"

#define BSCRIPT_MAX_NAME_LENGTH 32

typedef enum bscript_exception_type {
    no_memory, empty_value, unsupported_operation, syntax_error
} BScriptExceptionType;

typedef struct bscript_exception {
    int id;
    char message[1024];
    int line;
    BScriptExceptionType type;
    BScriptValue * value_ptr;
} BScriptException;

const char * RESERVED_WORDS[] = {
    "var", 
    "if", 
    "else", 
    "true", 
    "false", 
    "empty", 
    "string", 
    "number", 
    "boolean", 
    "and", 
    "or",
    "for",
    "foreach",
    "to",
    "function",
    "typeof",
    "=", "==", "!=", "+", "-", "/", "*", "%", "<", "<=", ">", ">=",
    NULL
};

const char * RESERVED_OPERATOR_CHARS = "=+-/*%()[]!<>";

struct bscript_value;

char ** BScriptReadLineTokens(char * script, size_t * offset, size_t * tokens_count_ptr);

BScriptException * BScriptCreateException(char * message, int line, BScriptValue * value_ptr, BScriptExceptionType type);

bool BScriptCharIsReservedOperatorChar(char c);
bool BScriptTokenIsReservedKeyword(char * token_string);
bool BScriptTokenIsNumericConstant(char * token_string);
bool BScriptTokenIsBooleanConstant(char * token_string);
bool BScriptTokenIsStringConstant(char * token_string);
bool BScriptTokenIsValidVariableName(char * token_string);

#endif