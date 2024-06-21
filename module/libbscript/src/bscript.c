#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bglobal.h"
#include "bscript.h"

char ** BScriptReadLineTokens(char * script, size_t * offset, size_t * tokens_count_ptr)
{
    if (!script || script[0] == 0 || !offset || !tokens_count_ptr) return NULL;
    size_t i = *offset;
    size_t tokens_array_size = 8, t = 0;
    char ** tokens_array = (char **) malloc(sizeof(char *) * tokens_array_size);
    if (!tokens_array) return NULL;
    bool valid_tokens = true;
    while (script[i] != 0 && script[i] != '\n' && script[i] != 0 && valid_tokens) {
        while(isblank(script[i])) i++;
        size_t j = i;
        bool quoted = false;
        int quote_count = 0;
        while (script[i] != 0 && (((!isblank(script[i]) && !BScriptCharIsReservedOperatorChar(script[i])) && !quoted) || (quoted && quote_count < 2))) {
            if (script[i] == '"') {
                if (i == 0 || script[i - 1] != '\\') {
                    quoted = !quoted;
                    quote_count++;
                }
            }
            i++;
        }
        if (BScriptCharIsReservedOperatorChar(script[i]) && j == i) i++;
        if (valid_tokens) {
            tokens_array[t] = (char *) malloc((i - j) + 1);
            if (!tokens_array[t]) valid_tokens = false;
            else {
                if (t == tokens_array_size) {
                    tokens_array = (char **) realloc(tokens_array, sizeof(char *) * (tokens_array_size + 8));
                    if (!tokens_array) valid_tokens = false;
                    else tokens_array_size += 8;
                }
                if (valid_tokens) {
                    strncpy(tokens_array[t], script + j, (int) i - j);
                    tokens_array[t][i-j] = 0;
                    t++;
                }
            }
        }
    }
    *offset = i;
    *tokens_count_ptr = t;
    return tokens_array;
}

BScriptException * BScriptCreateException(char * message, int line, BScriptValue * value_ptr, BScriptExceptionType type)
{
    BScriptException * ex = (BScriptException *) malloc(sizeof(BScriptException));
    if (ex) {
        ex->id = BSCRIPT_DATA_STRUCT_ID;
        ex->line = line;
        strncpy(ex->message, message, 1024);
        ex->type = type;
        ex->value_ptr = value_ptr;
    }
    return ex;
}

bool BScriptCharIsReservedOperatorChar(char c)
{
    int i = 0;
    bool found = false;
    while (RESERVED_OPERATOR_CHARS[i] != 0 && !found) {
        if (c == RESERVED_OPERATOR_CHARS[i]) found = true;
        i++;
    }
    return found;
}

bool BScriptTokenIsReservedKeyword(char * token_string)
{
    size_t i = 0;
    bool found = false;
    while (RESERVED_WORDS[i] && !found) {
        const char * RESERVED_WORD = RESERVED_WORDS[i];
        if (strcmp(RESERVED_WORD, token_string) == 0) found = true;
        i++;
    }
    return found;
}

bool BScriptTokenIsNumericConstant(char * token_string)
{
    if (!token_string || token_string[0] == 0) return false;
    bool numeric = true;
    size_t i = 0, dp_count = 0;
    while (token_string[i] != 0 && numeric) {
        numeric = isdigit(token_string[i]);
        if (token_string[i] == '.') dp_count++;
        if (dp_count > 1) numeric = false;
        i++;
    }
    return numeric;
}

bool BScriptTokenIsBooleanConstant(char * token_string)
{
    if (!token_string || token_string[0] == 0) return false;
    return strcmp(token_string,"true") == 0 || strcmp(token_string,"false") == 0;
}

bool BScriptTokenIsStringConstant(char * token_string)
{
    if (!token_string || token_string[0] == 0) return false;
    size_t quote_count = 0;
    size_t i = 0;
    if (token_string[i] != '"') return false;
    while (token_string[i] != 0) {
        if (token_string[i] == '"') {
            if (i == 0 || token_string[i - 1] != '\\') {
                quote_count++;
            }
        }
        i++;
    }
    return quote_count == 2;
}

bool BScriptTokenIsValidVariableName(char * token_string)
{
    if (!token_string || token_string[0] == 0) return false;
    size_t i = 0;
    while (token_string[i] != 0 && isblank(token_string[i])) i++;
    if (token_string[i] == 0) return false;
    if (token_string[i] >= '0' && token_string[i] <= '9') return false;
    size_t underscores = 0, j = i;
    while (token_string[i] != 0 && !BScriptCharIsReservedOperatorChar(token_string[i]) && !isblank(token_string[i]) && (isalpha(token_string[i]) || token_string[i] == '_')) {
        if (token_string[i] == '_') underscores++;
        i++;
    }
    if (i - j == underscores) return false;
    if (token_string[i] == '=' || token_string[i] == 0) return !BScriptTokenIsReservedKeyword(token_string + j);
    while (token_string[i] != 0 && isblank(token_string[i])) i++;
    if (token_string[i] != 0) return false;
    return true;
}