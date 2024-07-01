#include <stdio.h>

#include "bglobal.h"
#include "bboolean.h"
#include "bvalue.h"

BScriptBoolean * BScriptCreateBoolean(bool value);
void BScriptFreeBoolean(BScriptBoolean * boolean);
bool BScriptBooleanValueAsBoolean(BScriptValue *value);
double BScriptBooleanValueAsNumber(BScriptValue *value);
char * BScriptBooleanValueAsString(BScriptValue *value);
bool BScriptBooleanAddValueToArray(BScriptValue * array_value, BScriptValue * value_to_add);
BScriptValue * BScriptBooleanValuePlusOperation(BScriptValue * value1, BScriptValue * value2);

/// @brief Creates and returns a pointer to a new boolean BScriptValue data structure.  The boolean value passed to this 
///        function is copied as the boolean value stored in the newly created data structure.
/// @param boolean_value Value to store in the created BScriptValue data structure.
/// @return Pointer to the created data structure or NULL if the data structure could not be created.
BScriptValue * BScriptCreateBooleanValue(bool boolean_value)
{
    // Create a default BScriptValue data structure.
    BScriptValue * value = BScriptCreateValue(BScriptTypeBoolean);
    // Check to make sure the data structure exists.
    if (value) {
        // Populate the method pointers with boolean specific functions.
        value->methods.free = BScriptFreeBooleanValue;
        value->methods.typeAsString = BScriptBooleanValueGetTypeAsString;

        value->methods.plusOperator = BScriptBooleanValuePlusOperation;
        value->methods.valueAsString = BScriptBooleanValueAsString;
        value->methods.valueAsNumber = BScriptBooleanValueAsNumber;
        value->methods.valueAsBoolean = BScriptBooleanValueAsBoolean;

        value->methods.addToArray = BScriptBooleanAddValueToArray;

        // Create a BScriptBoolean data structure and set its value to the one passed to this function.
        value->data.boolean = BScriptCreateBoolean(boolean_value);
    }
    // Return a pointer to the created BScriptValue data structure, or NULL if no data structure was created.
    return value;
}

BScriptBoolean * BScriptCreateBoolean(bool value)
{
    // Try to create a BScriptBoolean data structure.
    BScriptBoolean * boolean = (BScriptBoolean *) malloc(sizeof(BScriptBoolean));
    
    if (boolean) {
        // If the data structure was created successfully, we can populate it with the standard structure id and
        // the boolean value passed into this function.
        boolean->id = BSCRIPT_DATA_STRUCT_ID;
        boolean->value = value;
    }

    // Return a pointer to the created data structure, or NULL if one wasn't created.
    return boolean;
} /*    This is a private function which is used to create a secondary data structure (BScriptBoolean) which itself holds the
        value passed to the function BScriptCreateBooleanValue.  The BScriptValue data structure created using that function
        references the secondary data structure using it's data field (the reference is stored in data.boolean as the data
        field is a union).*/

void BScriptFreeBoolean(BScriptBoolean * boolean)
{
    // Make sure we have a valid data structure, if not return immediately.
    if (!boolean || boolean->id != BSCRIPT_DATA_STRUCT_ID) return;

    // We do have a valid BScriptBoolean data structure, so reset the data structure values to their defaults and free any allocated memory.
    boolean->id = 0;
    boolean->value = false;
    free(boolean);
} /*    This is a private function which is used to destroy a BScriptBoolean data structure.
        Usually this function is called as part of the function below, BScriptFreeBooleanValue, which releases all memory
        allocated to a BScriptValue data structure. */

/// @brief Frees any memory allocated to the child BScriptBoolean data structure referenced by the BScriptValue data structure whose value was
///         passed to this function.  This method is not normally called directly and is usually referenced by the BScriptValue data structure's
///         method member methods.freeFunction.  When the parent BScriptValue data structure is freed, this method called via methods.freeFunction().
/// @param value Pointer to a valid BScriptValue data structure.
/// @return True if the memory was freed successfully, or false if the calling method tried to free an area of memory that wasn't a BScriptValue
///         data structure.
bool BScriptFreeBooleanValue(BScriptValue * value)
{
    // CHeck to make sure the calling function has passed a pointer to a valid boolean BScriptValue data structure.  If not return false.
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean) return false;

    // The data structure is valid, so we can reset and release 
    if (value->data.boolean) {
        BScriptFreeBoolean(value->data.boolean);
        value->data.boolean = NULL;
    }
    return true;
}

/// @brief This function is not normally called directly, but is referenced by a parent BScriptValue data structure under its methods member
///         (methods.valueAsBoolean)
/// @param value Pointer to the BScriptValue data structure.
/// @return Returns a boolean value, either the stored boolean value, or false if the given BScriptValue data structure pointer is not valid.
bool BScriptBooleanValueAsBoolean(BScriptValue *value)
{
    // Check to make sure the given pointer points to a valid BScriptValue data structure that holds a boolean value and that value isn't empty.
    // If these checks aren't passed, return false.
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean || value->is_empty) return false;
    // If the passed pointer is valid, return true or false based on the following criteria:
    // a) If the value data structure is for a single value, return the value.
    // b) If the value data structure is an array of booleans, return true if the array length is greater than zero (0).
    return !value->is_array ? value->data.boolean->value : value->array_length > 0;
}

/// @brief This function is not normally called directly, but is referenced by a parent BScriptValue data structure under its methods member
///         (methods.valueAsNumber)
/// @param value Pointer to the BScriptValue data structure.
/// @return Returns a double value, either the stored double value or zero if the given BScriptValue data structure pointer is not valid.
double BScriptBooleanValueAsNumber(BScriptValue *value)
{
    // Check to make sure the given pointer points to a valid BScriptValue data structure that holds a numeric value and that value isn't empty.
    // If these checks aren't passed, return false.
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean || value->is_empty) return 0.0;
    // If the passed pointer is valid, return the stored numerical value or zero (0) based on the following criteria:
    // a) If the value data structure is for a single value, return the value.
    // b) If the value data structure is an array of numbers, return 0.
    return !value->is_array? value->data.boolean->value ? 1.0 : 0.0 : 0;
}

/// @brief Returns a char string (zero terminated), containing either a single number or a list of number separated by commas if the given
///         BScriptValue pointer points to an array value.
/// @param value Pointer to a BScriptValue data structure.
/// @return A char string holding the textual equivalent of one or more numbers.
char * BScriptBooleanValueAsString(BScriptValue *value)
{
    // Check to make sure the given pointer points to a valid BScriptValue data structure that holds a numeric value and that value isn't empty.
    // If these checks aren't passed, return false.
    if (!value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean || value->is_empty) return NULL;

    // return_string will hold the final string of number(s).
    char * return_string = NULL;

    // Check to see if the BScriptValue data structure is an array.
    if (!value->is_array) {
        // It isn't so create a string that only holds a single number value.
        char * boolean_string_value = (char *) malloc(6);
        if (boolean_string_value) {
            size_t string_length = sprintf(boolean_string_value, "%s", value->data.boolean->value ? "true" : "false");
            boolean_string_value[string_length] = 0;
        }
        return_string = boolean_string_value;
    } else {
        // If is an array, so create a string that holds each number value in the array, separated by commas.
        if (value->array_length < 1) return NULL;
        return_string = (char *) malloc(6 * value->array_length);
        if (return_string) {
            size_t w = 0;
            for (size_t r = 0; r < value->array_length; r++) {
                bool boolean_value = value->data.array[r]->methods.valueAsBoolean(value->data.array[r]);
                size_t string_length = sprintf(return_string + w, "%s", boolean_value ? "true" : "false");
                w += string_length;
                return_string[w++] = r < value->array_length - 1 ? ',' : 0;
            }
        }
    }

    // Return a pointer to the created string of number(s) or NULL if the string couldn't be created.
    return return_string;
}

/// @brief Adds a boolean BScriptValue data structure to the end of a boolean array stored within another BScriptValue data structure.
/// @param array_value Pointer to the BScriptValue data structure that holds the array of booleans to add the value to.
/// @param value_to_add Pointer to the BScriptValue data structure that holds the boolean value to be added to the array.
/// @return Returns true if the value was added to the array or false otherwise.
bool BScriptBooleanAddValueToArray(BScriptValue * array_value, BScriptValue * value_to_add)
{
    // Make sure suitable BScriptValue data structures have been passed into this function.  If not return false and exit immediately.
    if (!array_value || array_value->id != BSCRIPT_DATA_STRUCT_ID || !array_value->is_array || array_value->type != BScriptTypeBoolean) return false;
    if (!value_to_add || value_to_add->id != BSCRIPT_DATA_STRUCT_ID || value_to_add->is_empty) return false;
    // Check to see if the BScriptValue pointer points to a single value or an array of boolean values.
    if (value_to_add->is_array) {
        // It looks like we've got to add an array of values to the other array.
        if (array_value->array_max_size - array_value->array_length < value_to_add->array_length) {
            // Make sure the receiving array has enough space for the values we're going to copy over.
            while (array_value->array_max_size - array_value->array_length < value_to_add->array_length) BScriptResizeValueArray(array_value);
            // Copy over the array of boolean values from 'value_to_add' to the array held in 'array_value'.
            for (size_t r = 0; r < value_to_add->array_length; r++) {
                BScriptValue * boolean_value = BScriptCreateBooleanValue(value_to_add->methods.valueAsBoolean(value_to_add->data.array[r]));
                array_value->data.array[array_value->array_length++] = boolean_value;
            }
        }
    } else {
        // Looks like we're only adding one value, so add it.
        if (array_value->array_length == array_value->array_max_size) BScriptResizeValueArray(array_value);
        array_value->data.array[array_value->array_length++] = BScriptCreateBooleanValue(value_to_add->methods.valueAsBoolean(value_to_add));
    }
    // Return true to indicate the value(s) were added.
    return true;
}

/// @brief This method is not normally called directly, but is usually invoked through a BScriptValue data structure's plusOperator 
///         (methods.plusOperator) member.  The result of calling this function against two valid BScriptValues varies depending on 
///         the content of each data structure.  See the documentation for @return for more details.
/// @param value1 First BScriptValue data structure.  It's actual value (string, number, boolean) will be translated into an appropriate boolean value.
/// @param value2 Second BScriptValue data structure.  
/// @return Depending on the contents of the BSCriptValue data structures involved, the returned BSCriptValue data structure will either contain:
///         a) one boolean value which equates to performing a logical and (&) operation using the two boolean values held in value1 and value2.
///         b) A pointer to the same data structure as 'value1' but with the boolean value contents of 'value2' appended to the end.
///         c) A completely new BScriptValue data structure holding an array that is a combination of the contents of both 'value1' and 'value2's
///             boolean data.
BScriptValue * BScriptBooleanValuePlusOperation(BScriptValue * value1, BScriptValue * value2)
{
    if (!value1 || !value2) return NULL;
    if (value1->id != BSCRIPT_DATA_STRUCT_ID || value1->type != BScriptTypeBoolean) return NULL;
    if (value2->id != BSCRIPT_DATA_STRUCT_ID) return NULL;
    if (value1->is_empty && value2->is_empty) return NULL;
    if (value1->is_empty && !value2->is_empty) return value2;
    if (!value1->is_empty && value2->is_empty) return value1;

    if (!value1->is_array && !value2->is_array) {
        bool result = value1->data.boolean->value & value2->methods.valueAsBoolean(value2);
        return BScriptCreateBooleanValue(result);
    }
    if (value1->is_array) {
        value1->methods.addToArray(value1, value2);
        return value1;
    } else if (value2->is_array) {
        return BScriptCombineValuesIntoArray(value1, value2);
    }
    return NULL;
}

char * BScriptBooleanValueGetTypeAsString(BScriptValue * value)
{
    return !value || value->id != BSCRIPT_DATA_STRUCT_ID || value->type != BScriptTypeBoolean ? NULL : "Boolean";
}