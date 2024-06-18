#include <stdbool.h>
#include <string.h>

#include "bglobal.h"
#include "bmemory.h"
#include "bvariable.h"

bool BScriptAddVariableToMemory(BScriptVariable * var, BScriptMemory * memory)
{
    if (!var || var->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (!memory || memory->id != BSCRIPT_DATA_STRUCT_ID) return false;
    if (var->name == NULL) return false;
    const int BLOCK_INDEX = var->name->hash % 256;
    bool found = false;
    for (int i = 0; i < memory->variable_blocks[BLOCK_INDEX].wp && !found; i++) {
        if (memory->variable_blocks[BLOCK_INDEX].array[i] && memory->variable_blocks[BLOCK_INDEX].array[i]->name->hash == var->name->hash) {
            if (strcmp(memory->variable_blocks[BLOCK_INDEX].array[i]->name->data, var->name->data) == 0) found = true;
        }
    }
    if (!found) {
        if (memory->variable_blocks[BLOCK_INDEX].wp < 256) memory->variable_blocks[BLOCK_INDEX].array[memory->variable_blocks[BLOCK_INDEX].wp++] = var;
    }
    return !found;
}
