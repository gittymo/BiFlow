#ifndef COM_PLUS_MEVANSPN_BSCRIPT_NUMBER
#define COM_PLUS_MEVANSPN_BSCRIPT_NUMBER

#include <stdlib.h>
#include <stdbool.h>

#include "bglobal.h"

typedef struct bscript_number {
    int id;
    double value;
    int decimal_places;
} BScriptNumber;

struct bscript_value * BScriptCreateNumberValue(double value, int decimal_places);
bool BScriptFreeNumberValue(struct bscript_value * value);

char * BScriptNumberAsCharString(struct bscript_value * number_value);
#endif