#ifndef COM_PLUS_MEVANSPN_BSCRIPT_VARIABLE_BLOCK
#define COM_PLUS_MEVANSPN_BSCRIPT_VARIABLE_BLOCK

#include "bvariable.h"

typedef struct bscript_variables_block {
    int id;
    BScriptVariable * array[256];
    int wp;
} BScriptVariableBlock;

#endif