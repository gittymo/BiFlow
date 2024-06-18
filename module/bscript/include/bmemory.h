#ifndef COM_PLUS_MEVANSPN_BSCRIPT_MEMORY
#define COM_PLUS_MEVANSPN_BSCRIPT_MEMORY

#include "bvariable.h"
#include "bvariable_block.h"

typedef struct bscript_memory {
    int id;
    BScriptVariableBlock variable_blocks[256];
    BScriptVariable * anon_constants[1024];
    int anon_constants_wp;
} BScriptMemory;

bool BScriptAddVariableToMemory(BScriptVariable * var, BScriptMemory * memory);

#endif