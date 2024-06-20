#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bglobal.h"

int BScriptGetHash(char * string)
{
    int primes[] = {31, 131, 241, 139, 47};
    int hash = 0;
    size_t i = 0;
    while (string[i]) {
        int prime_index = i % 5;
        int next_prime_index = (i + 1) % 5;
        hash = primes[prime_index] + (string[i] * primes[next_prime_index]);
        i++;
    }
    return hash;
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