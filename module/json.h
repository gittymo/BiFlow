#ifndef COM_PLUS_MEVANSPN_BIFLOW_JSON
#define COM_PLUS_MEVANSPN_BIFLOW_JSON

#include <stdbool.h>
#include <stdlib.h>

#define JSON_MAX_NAME_LENGTH 256
#define JSON_MAX_STRING_VALUE_LENGTH 65536

typedef enum json_value_type {
    JSONValueType_String, 
    JSONValueType_Number, 
    JSONValueType_Boolean, 
    JSONValueType_Null, 
    JSONValueType_Array, 
    JSONValueType_Object, 
    JSONValueType_NameValuePair,
    JSONValueType_Undefined
} JSONValueType;

typedef struct json_element JSONElement;

JSONElement * JSONCreateStringElement(char * string);
JSONElement * JSONCreateNumberElement(double number);
JSONElement * JSONCreateBooleanElement(bool boolean);
JSONElement * JSONCreateNullElement();
JSONElement * JSONCreateArrayElement();
JSONElement * JSONCreateObjectElement();
JSONElement * JSONCreateNameValuePairElement(char * name, JSONElement * child_element);

bool JSONIsStringElement(JSONElement * element);
bool JSONIsNumberElement(JSONElement * element);
bool JSONIsBooleanElement(JSONElement * element);
bool JSONIsNullElement(JSONElement * element);
bool JSONIsArrayElement(JSONElement * element);
bool JSONIsObjectElement(JSONElement * element);
bool JSONIsNameValuePairElement(JSONElement * element);
bool JSONIsContainerElement(JSONElement * element);
JSONValueType JSONGetElementValueType(JSONElement * element);

bool JSONCanAddChildToElement(JSONElement * child_element, JSONElement * parent_element);
bool JSONAddChildToElement(JSONElement * child_element, JSONElement * parent_element);
JSONElement ** JSONGetChildElementsArray(JSONElement * container_element, size_t * array_size_ptr);
size_t JSONGetChildElementsArraySize(JSONElement * container_element);

double JSONGetValueAsDouble(JSONElement * element);
long JSONGetValueAsLong(JSONElement * element);
bool JSONGetValueAsBoolean(JSONElement * element);
char * JSONGetValueAsString(JSONElement * element, int dp);

bool JSONSetValueUsingDouble(JSONElement * element, double value, int dp);
bool JSONSetValueUsingLong(JSONElement * element, long value);
bool JSONSetValueUsingBoolean(JSONElement * element, bool value);
bool JSONSetValueUsingString(JSONElement * element, char * string);

JSONElement * JSONReadElementFromFile(char * filename);
bool JSONWriteElementToFile(JSONElement * e, char * filename);
bool JSONFreeElement(JSONElement * element);

#endif