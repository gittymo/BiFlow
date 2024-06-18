#include <string.h>

#include "bglobal.h"
#include "bvariable.h"
#include "bstring.h"
#include "bboolean.h"
#include "bnumber.h"

BScriptVariable * BScriptCreateVariable(char * name, char * value, BScriptValueType variable_type)
{
    if (!name || name[0] == 0) return NULL;
    BScriptVariable * var = (BScriptVariable *) malloc(sizeof(BScriptVariable));
    if (var) {
        var->id = BSCRIPT_DATA_STRUCT_ID;
        if (BScriptTokenIsValidVariableName(name)) {
            var->name = BScriptCreateString(name);
            if (!var->name) {
                BScriptFreeVariable(var);
                return NULL;
            }
        } else {
            BScriptFreeVariable(var);
            return NULL;
        }

        BScriptValue * script_value = NULL;
        
        switch (variable_type) {
            case BScriptTypeString : {
                script_value = BScriptCreateStringValue(value);
            } break;
            case BScriptTypeNumber : {
                if (BScriptTokenIsNumericConstant(value)) {
                    script_value = BScriptCreateNumberValue(atof(value),3);
                } else {
                    BScriptFreeVariable(var);
                    return NULL;
                }
            } break;
            case BScriptTypeBoolean : {
                if (BScriptTokenIsBooleanConstant(value)) {
                    if (strcmp(value,"true") == 0 || strcmp(value,"false") == 0) {
                        script_value = BScriptCreateBooleanValue(strcmp(value,"true") == 0 ? true : false);
                    } else {
                        BScriptFreeVariable(var);
                        return NULL;
                    }
                } else {
                    BScriptFreeVariable(var);
                    return NULL;
                }
            } break;
            case BScriptTypeUndefined : {
                BScriptFreeVariable(var);
                return NULL;
            }
        }

        var->value = script_value;
    }
    return var;
}

bool BScriptFreeVariable(BScriptVariable * var)
{
    if (!var || var->id != BSCRIPT_DATA_STRUCT_ID) return false;
    var->id = 0;
    if (var->name) {
        BScriptFreeString(var->name);
        var->name = NULL;
    }
    if (var->value) {
        BScriptFreeValue(var->value);
        var->value = NULL;
    }
    free(var);
    return true;
}