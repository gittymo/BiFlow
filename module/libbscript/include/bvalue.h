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
    char * (*typeAsString)(struct bscript_value *);

    struct bscript_value * (*plusOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*minusOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*multiplyOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*divideOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*modulusOperator)(struct bscript_value *, struct bscript_value *);

    struct bscript_value * (*andOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*orOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*notOperator)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*xorOperator)(struct bscript_value *, struct bscript_value *);

    bool (*equalOperator)(struct bscript_value *, struct bscript_value *);
    bool (*notEqualOperator)(struct bscript_value *, struct bscript_value *);
    bool (*greaterThanOperator)(struct bscript_value *, struct bscript_value *);
    bool (*lessThanOperator)(struct bscript_value *, struct bscript_value *);

    bool (* valueAsBoolean)(struct bscript_value *);
    double (* valueAsNumber)(struct bscript_value *);
    char * (* valueAsString)(struct bscript_value *);

    bool (* addToArray)(struct bscript_value *, struct bscript_value *);
    struct bscript_value * (*createCombinedArray)(struct bscript_value *, struct bscript_value *);

    bool (* free)(struct bscript_value *);
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

bool BScriptValueEqualsOperator(BScriptValue * val1, BScriptValue * val2);

#endif