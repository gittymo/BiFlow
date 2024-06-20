#include "libmem.h"
#include <string.h>
int main(int argc, char ** argv)
{
    const int TESTS = 17;
    char * text[TESTS];
    for (int i = 0; i < TESTS; i++) {
        if (i % 2 == 0) {
            text[i] = LibMemAlloc(4096, (void **) &text[i]);
        } else {
            text[i] = LibMemAllocLocal(4096, (void **) &text[i]);
        }
        if (i == 2) {
            char * str = text[i];
            strcpy(str, "Hello world!");
        }
    }

    LibMemFreeLocals();

    /*for (int i = 0; i < TESTS; i++) {
        LibMemFree(text[i]);
    }*/

    exit(EXIT_SUCCESS);
}