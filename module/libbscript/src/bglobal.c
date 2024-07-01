#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
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