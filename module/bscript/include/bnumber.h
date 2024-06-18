#ifndef COM_PLUS_MEVANSPN_BSCRIPT_NUMBER
#define COM_PLUS_MEVANSPN_BSCRIPT_NUMBER

#include <stdlib.h>
#include <stdbool.h>

typedef struct bscript_number {
    int id;
    double value;
    int decimal_places;
} BScriptNumber;

struct bscript_value * BScriptCreateNumberValue(double value, int decimal_places);
bool BScriptFreeNumberValue(struct bscript_value * value);
BScriptNumber * BScriptCreateNumber(double value, int decimal_places);
void BScriptFreeNumber(BScriptNumber * number);
char * BScriptGetNumberValueAsCharString(struct bscript_value * number_value, size_t * string_length_ptr);

#endif