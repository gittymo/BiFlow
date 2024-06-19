#ifndef COM_PLUS_MEVANSPN_BSCRIPT_VALUE
#define COM_PLUS_MEVANSPN_BSCRIPT_VALUE

#include <stdlib.h>

#include "bglobal.h"
#include "bstring.h"
#include "bnumber.h"
#include "bboolean.h"

typedef enum bscript_value_types {
    BScriptTypeUndefined, BScriptTypeString, BScriptTypeNumber, BScriptTypeBoolean
} BScriptValueType;

typedef struct bscript_value_interface {
    struct bscript_value * (*plusOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*minusOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*multiplyOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*divideOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*equalsOperator)(struct bscript_value *, struct bscript_value *);

    bool (* valueAsBoolean)(struct bscript_value *);
    double (* valueAsNumber)(struct bscript_value *);
    char (* valueAsString)(struct bscript_value *);

    bool (* addToArray)(struct bscript_value *, struct bscript_value *);

    bool (* freeFunction)(struct bscript_value *);
} BScriptValueInterface;

typedef struct bscript_value {
    int id;
    bool is_empty;
    union bscript_value_data {
        BScriptString * string;
        BScriptNumber * number;
        BScriptBoolean * boolean;
        struct bscript_value ** array;
    } data;
    BScriptValueType type;
    bool is_array;
    size_t array_length;
    size_t array_max_size;
    BScriptValueInterface methods;
} BScriptValue;

BScriptValue * BScriptCreateValue(BScriptValueType type);
bool BScriptFreeValue(BScriptValue * value);
bool BScriptResizeValueArray(BScriptValue * value);
BScriptValue * BScriptCombineValuesIntoArray(BScriptValue * left_value, BScriptValue * right_value);

#endif