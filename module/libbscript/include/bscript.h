#ifndef COM_PLUS_MEVANSPN_BSCRIPT
#define COM_PLUS_MEVANSPN_BSCRIPT

#include <stdlib.h>
#include "bvalue.h"

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

char ** BScriptReadLineTokens(char * script, size_t * offset, size_t * tokens_count_ptr);

BScriptException * BScriptCreateException(char * message, int line, BScriptValue * value_ptr, BScriptExceptionType type);

#endif