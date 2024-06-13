#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <sys/stat.h>

#include "libjson.h"

#define JSON_OUTPUT_BUFFER_BLOCK_SIZE (4 * JSON_MAX_STRING_VALUE_LENGTH)
#define JSON_ARRAY_BLOCK_SIZE 256
#define JSON_ELEMENT_ID (('J' << 24) + ('S' << 16) + ('O' << 8) + 'N')

typedef struct json_element {
    int id;
    union value_union {
        char * string;
        double number;
        bool boolean;
        struct json_element ** array;
        void * namevaluepair[2];
    } data;
    JSONValueType value_type;
    size_t length;
    size_t _size;
    int _hash;
} JSONElement;

typedef struct json_output_buffer {
    int id;
    char * data;
    size_t size;
    size_t wp;
    bool reused;
    size_t tab_pos;
    JSONElement * top_level_element;
} JSONOutputBuffer;

JSONElement * _JSONCreateElement()
{
    JSONElement * e = (JSONElement *) malloc(sizeof(JSONElement));
    if (e) {
        e->id = JSON_ELEMENT_ID;
        e->value_type = JSONValueType_Undefined;
        e->length = 0;
        e->_size = 0;
        e->_hash = 0;
    }
    return e;
}

bool JSONFreeElement(JSONElement * element)
{
    if (!element || element->id != JSON_ELEMENT_ID) return false;
    switch (element->value_type) {
        case JSONValueType_String : {
            if (element->data.string) {
                free(element->data.string);
                element->data.string = NULL;
            }
        } break;
        case JSONValueType_Boolean : {
            element->data.boolean = false;
        } break;
        case JSONValueType_Number : {
            element->data.number = 0;
        } break;
        case JSONValueType_Array :
        case JSONValueType_Object : {
            if (element->length > 0 && element->data.array) {
                for (size_t i = 0; i < element->length; i++) {
                    JSONFreeElement(element->data.array[i]);
                    element->data.array[i] = NULL;
                }
                free(element->data.array);
                element->data.array = NULL;
            }
        } break;
        case JSONValueType_NameValuePair : {
            if (element->data.namevaluepair[0]) {
                free(element->data.namevaluepair[0]);
                element->data.namevaluepair[0] = NULL;
            }
            if (element->data.namevaluepair[1]) {
                JSONFreeElement((JSONElement *) element->data.namevaluepair[1]);
                element->data.namevaluepair[1] = NULL;
            }
        } break;
        case JSONValueType_Null :
        case JSONValueType_Undefined : {
            // Nothing to do here!.
        } break;
        default: return false;
    }
    element->id = 0;
    element->length = 0;
    element->value_type = JSONValueType_Undefined;
    element->_size = 0;
    element->_hash = 0;
    free(element);
    return true;
}

bool _JSONElementIsValid(JSONElement * element)
{
    return element && element->id == JSON_ELEMENT_ID && element->value_type != JSONValueType_Undefined;
}

char * _JSONCopyString(char * string, size_t * string_copy_length_ptr)
{
    if (!string) return NULL;
    if (string_copy_length_ptr) *string_copy_length_ptr = 0;
    size_t l;
    for (l = 0; l < JSON_MAX_STRING_VALUE_LENGTH && string[l]; l++);
    char * string_copy = (char *) malloc(l + 1);
    if (string_copy) {
        strncpy(string_copy, string, l);
        if (string_copy_length_ptr) *string_copy_length_ptr = l;
    }
    return string_copy;
}

int _JSONCreateStringHash(char * string)
{
    int primes[7] = {31, 131, 241, 139, 47, 151, 269};
    int hash = 0;
    size_t i = 0;
    while (i < JSON_MAX_NAME_LENGTH && string[i] != 0) {
        size_t prime_index = i % 7;
        size_t next_prime_index = (i + 1) % 7;
        hash += primes[prime_index] + (string[i] * primes[next_prime_index]);
        i++;
    }
    return hash;
}

JSONValueType JSONGetElementValueType(JSONElement * element)
{
    return _JSONElementIsValid(element) ? element->value_type : JSONValueType_Undefined;
}

JSONElement * JSONCreateStringElement(char * string)
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_String;
        e->data.string = _JSONCopyString(string, &e->length);
        if (!e->data.string) {
            JSONFreeElement(e);
            e = NULL;
        }
    }
    return e;
}

bool JSONIsStringElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && element->value_type == JSONValueType_String;
}

JSONElement * JSONCreateNumberElement(double number)
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_Number;
        e->data.number = number;
    }
    return e;
}

bool JSONIsNumberElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && element->value_type == JSONValueType_Number;
}

JSONElement * JSONCreateBooleanElement(bool boolean)
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_Boolean;
        e->data.boolean = boolean;
    }
    return e;
}

bool JSONIsBooleanElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && element->value_type == JSONValueType_Boolean;
}

JSONElement * JSONCreateNullElement()
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_Null;
    }
    return e;
}

bool JSONIsNullElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && element->value_type == JSONValueType_Null;
}

JSONElement ** _JSONCreateContainerArray()
{
    return (JSONElement **) malloc(sizeof(JSONElement *) * JSON_ARRAY_BLOCK_SIZE);
}

bool _JSONResizeContainerArray(JSONElement * container)
{
    JSONElement ** new_array = (JSONElement**) realloc(container->data.array, container->_size + JSON_ARRAY_BLOCK_SIZE);
    if (!new_array) return false;
    else {
        container->data.array = new_array;
        container->_size += JSON_ARRAY_BLOCK_SIZE;
    }
    return true;
}

JSONElement * JSONCreateArrayElement()
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_Array;
        e->data.array = _JSONCreateContainerArray();
        if (!e->data.array) {
            JSONFreeElement(e);
            e = NULL;
        } else {
            e->_size = JSON_ARRAY_BLOCK_SIZE;
        }
    }
    return e;
}

JSONElement * JSONCreateObjectElement()
{
    JSONElement * e = _JSONCreateElement();
    if (e) {
        e->value_type = JSONValueType_Object;
        e->data.array = _JSONCreateContainerArray();
        if (!e->data.array) {
            JSONFreeElement(e);
            e = NULL;
        } else {
            e->_size = JSON_ARRAY_BLOCK_SIZE;
        }
    }
    return e;
}

bool _JSONAddElementToContainerElement(JSONElement * container, JSONElement * element)
{
    if (container->length == container->_size) {
        bool resized = _JSONResizeContainerArray(container);
        if (!resized) return false;
    }
    container->data.array[container->length++] = element;
    return true;
}

bool JSONIsContainerElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && (element->value_type == JSONValueType_Array || element->value_type == JSONValueType_Object);
}

bool JSONIsArrayElement(JSONElement * element)
{
    return JSONIsContainerElement(element) && element->value_type == JSONValueType_Array;
}

bool JSONIsObjectElement(JSONElement * element)
{
    return JSONIsContainerElement(element) && element->value_type == JSONValueType_Object;
}

JSONElement * JSONGetObjectElementMember(JSONElement * object_element, char * member_name)
{
    if (!JSONIsObjectElement(object_element) || !member_name || member_name[0] == 0) return NULL;
    int member_name_hash = _JSONCreateStringHash(member_name);

    JSONElement * found_element = NULL;
    for (size_t i = 0; i < object_element->length && !found_element; i++) {
        if (member_name_hash == object_element->data.array[i]->_hash) {
            if (strcmp(member_name, object_element->data.array[i]->data.namevaluepair[0])) {
                found_element = object_element->data.array[i];
            }
        }
    }

    return found_element;
}

bool JSONCanAddChildToElement(JSONElement * child_element, JSONElement * parent_element)
{
    return  _JSONElementIsValid(child_element) && 
            JSONIsContainerElement(parent_element) &&
            ((parent_element->value_type == JSONValueType_Array && child_element->value_type != JSONValueType_NameValuePair) ||
             (parent_element->value_type == JSONValueType_Object && child_element->value_type == JSONValueType_NameValuePair));
}

bool JSONAddChildToElement(JSONElement * child_element, JSONElement * parent_element)
{
    return JSONCanAddChildToElement(child_element, parent_element) ? _JSONAddElementToContainerElement(parent_element, child_element) : false;
}

JSONElement * JSONCreateNameValuePairElement(char * name, JSONElement * child_element)
{
    if (!name || name[0] == 0) return NULL;
    if (!_JSONElementIsValid(child_element)) return NULL;

    JSONElement * e = _JSONCreateElement();
    if (!e) return NULL;

    e->value_type = JSONValueType_NameValuePair;
    e->data.namevaluepair[0] = _JSONCopyString(name,NULL);
    e->_hash = _JSONCreateStringHash(e->data.namevaluepair[0]);
    if (!e->data.namevaluepair[0]) {
        JSONFreeElement(e);
        return NULL;
    }

    e->data.namevaluepair[1] = child_element;
    return e;
}

bool JSONIsNameValuePairElement(JSONElement * element)
{
    return _JSONElementIsValid(element) && element->value_type == JSONValueType_NameValuePair;
}

JSONOutputBuffer * _JSONResizeBuffer(JSONOutputBuffer * buff, size_t required_bytes)
{
    if (!buff || buff->id != JSON_ELEMENT_ID) return NULL;
    if (required_bytes == 0) return buff;
    const size_t BYTES_TO_RESIZE = required_bytes - (buff->size - buff->wp) > JSON_OUTPUT_BUFFER_BLOCK_SIZE ? required_bytes + JSON_OUTPUT_BUFFER_BLOCK_SIZE : JSON_OUTPUT_BUFFER_BLOCK_SIZE;
    if (required_bytes >= buff->size - buff->wp) {
        char * new_data_ptr = (char *) realloc(buff->data, buff->size + BYTES_TO_RESIZE);
        if (new_data_ptr) {
            buff->data = new_data_ptr;
            buff->size += BYTES_TO_RESIZE;
        } else {
            buff->id = 0;
            buff->size = buff->wp = 0;
            free(buff->data);
            buff->data = NULL;
            free(buff);
            return NULL;
        }
    }
    return buff;
}

JSONOutputBuffer * _JSONCreateBuffer() 
{
    JSONOutputBuffer * ob = (JSONOutputBuffer *) malloc(sizeof(JSONOutputBuffer));
    if (ob) {
        ob->data = (char *) malloc(JSON_OUTPUT_BUFFER_BLOCK_SIZE);
        if (!ob->data) {
            free(ob);
            ob = NULL;
        } else {
            ob->id = JSON_ELEMENT_ID;
            ob->reused = false;
            ob->size = JSON_OUTPUT_BUFFER_BLOCK_SIZE;
            ob->wp = 0;
            ob->tab_pos = 0;
            ob->top_level_element = NULL;
        }
    }
    return ob;
}

void _JSONFreeBuffer(JSONOutputBuffer * buff)
{
    if (!buff || buff->id != JSON_ELEMENT_ID) return;
    buff->id = 0;
    if (buff->data) {
        free(buff->data);
        buff->data = NULL;
    }
    buff->size = buff->wp = 0;
    buff->tab_pos = 0;
    buff->reused = false;
    buff->top_level_element = NULL;
    free(buff);
}

JSONOutputBuffer * _JSONWriteElementToBuffer(JSONElement * element, JSONOutputBuffer * existing_output_buffer_ptr, size_t start_tab_pos)
{
    if (!_JSONElementIsValid(element)) return existing_output_buffer_ptr;
    if (existing_output_buffer_ptr && existing_output_buffer_ptr->id != JSON_ELEMENT_ID) return existing_output_buffer_ptr;
    if (existing_output_buffer_ptr && !existing_output_buffer_ptr->top_level_element) {
        if (JSONIsContainerElement(element)) existing_output_buffer_ptr->top_level_element = element;
        else return existing_output_buffer_ptr;
    }

    JSONOutputBuffer * buff = existing_output_buffer_ptr ? existing_output_buffer_ptr : _JSONCreateBuffer();
    if (!buff) return NULL;

    buff->wp = existing_output_buffer_ptr ? existing_output_buffer_ptr->wp : 0;
    buff->reused = existing_output_buffer_ptr ? true : false;
    buff->tab_pos = existing_output_buffer_ptr ? existing_output_buffer_ptr->tab_pos : start_tab_pos >= 0 ? start_tab_pos : 0;

    size_t temp_buffer_length = JSON_MAX_NAME_LENGTH + 2 + JSON_MAX_STRING_VALUE_LENGTH;
    char * temp_buffer = (char *) malloc(temp_buffer_length);
    if (!temp_buffer) return buff;

    size_t length = 0;
    if (element->value_type == JSONValueType_NameValuePair) {
        length += sprintf(temp_buffer, "\"%s\":", (char *) element->data.namevaluepair[0]);
        element = element->data.namevaluepair[1];
    }

    switch (element->value_type) {
        case JSONValueType_String : {
            length += sprintf(temp_buffer + length, "\"%s\"", element->data.string);
        } break;
        case JSONValueType_Number : {
            if ((int) element->data.number == element->data.number) {
                length += sprintf(temp_buffer + length, "%li", (long int) element->data.number);
            } else {
                length += sprintf(temp_buffer + length, "%f", element->data.number);
            }
        } break;
        case JSONValueType_Null : {
            length += sprintf(temp_buffer + length, "%s", "null");
        } break;
        case JSONValueType_Boolean : {
            length += sprintf(temp_buffer + length, "%s", element->data.boolean ? "true" : "false");
        } break;
        case JSONValueType_Array : 
        case JSONValueType_Object : {
            JSONOutputBuffer * tob = NULL;
            for (size_t i = 0; i < element->length; i++) {
                tob = _JSONWriteElementToBuffer(element->data.array[i], tob, buff->tab_pos + 1);
            }
            if (!tob) return buff;
            if (temp_buffer_length < length + tob->wp) {
                temp_buffer = (char *) realloc(temp_buffer, length + tob->wp);
                if (!temp_buffer) return buff;
            }
            size_t t = length;
            temp_buffer[t++] = element->value_type == JSONValueType_Array ? '[' : '{';
            temp_buffer[t++] = '\n';
            strncpy(temp_buffer + t, tob->data, tob->wp);
            t += tob->wp;
            temp_buffer[t++] = '\n';
            for (size_t i = 0; i < buff->tab_pos; i++) temp_buffer[t++] = '\t';
            temp_buffer[t++] = element->value_type == JSONValueType_Array ? ']' : '}';
            length = t;
            _JSONFreeBuffer(tob);
        } break;
        default:{}
    }

    temp_buffer[length] = 0;
    
    const size_t EXTRA_CHAR_COUNT = (buff->reused ? 2 : 0) + buff->tab_pos;

    if (length + EXTRA_CHAR_COUNT >= buff->size - buff->wp) buff = _JSONResizeBuffer(buff, length + EXTRA_CHAR_COUNT);
    if (!buff) return existing_output_buffer_ptr;

    if (buff->reused) {
        buff->data[buff->wp++] = ',';
        buff->data[buff->wp++] = '\n';
    }
    for (size_t i = 0; i < buff->tab_pos; i++) buff->data[buff->wp++] = '\t';
    strncpy(buff->data + buff->wp, temp_buffer, length);
    buff->wp += length;

    free(temp_buffer);

    return buff;
}

bool _JSONWriteBufferToFile(JSONOutputBuffer * buff, char * filename)
{
    if (!buff || buff->id != JSON_ELEMENT_ID || !buff->top_level_element || !filename || filename[0] == 0) return false;
    FILE * outfile = fopen(filename,"w");
    fwrite(buff->data, 1, buff->wp, outfile);
    fclose(outfile);
    return true;
}

bool JSONWriteElementToFile(JSONElement * e, char * filename)
{
    if (!_JSONElementIsValid(e)) return false;
    JSONOutputBuffer * ob = _JSONWriteElementToBuffer(e, NULL, 0);
    return ob ? _JSONWriteBufferToFile(ob, filename) : false;
}

bool _JSONCharIsValid(char * string, size_t offset, size_t search_start_offset, char petc) {
    if (!string || string[0] == 0 || offset < search_start_offset) return false;
    char c = string[offset];
    bool quoted = false;
    size_t i = search_start_offset;
    while (string[i] != 0 && i < offset) {
        if (string[i] == '"' && (i == 0 || string[i - 1] != '\\')) {
            quoted = !quoted;
        }
        i++;
    }
    if (i != offset) return false;

    bool is_valid = quoted || ((isalnum(c) || c == '"') && c != petc);
    return is_valid;
}

JSONElement * _JSONGetElementFromString(char * string, size_t * offset, char petc)
{
    JSONElement * e = _JSONCreateElement();
    if (!string || string[0] == 0) return e;

    size_t i = *offset;
    
    while ((isblank(string[i]) || string[i] == '\n')) i++;
    
    if (string[i] == '[') {
        i++;
        JSONElement * ne = JSONCreateArrayElement();
        while (string[i] != ']' && string[i] != '0') {
            JSONElement * ce = _JSONGetElementFromString(string, &i, ']');
            if (ce->value_type != JSONValueType_Undefined && ce->value_type != JSONValueType_NameValuePair) {
                JSONAddChildToElement(ne, ce);
            } else {
                JSONFreeElement(ne);
                JSONFreeElement(e);
                e = NULL;
            }
        }
        if (e->value_type == JSONValueType_Undefined) {
            JSONFreeElement(e);
            e = ne;
        } else {
            JSONAddChildToElement(e, ne);
        }
        i++;
    } else if (string[i] == '{') {
        i++;
        JSONElement * ne = JSONCreateObjectElement();
        while (string[i] != '}' && string[i] != '0') {
            JSONElement * ce = _JSONGetElementFromString(string, &i, '}');
            if (ce->value_type == JSONValueType_NameValuePair) {
                JSONAddChildToElement(ne, ce); /* ce->value_type == JSONValueType_Undefined || */
            } else {
                JSONFreeElement(ne);
                JSONFreeElement(e);
                e = NULL;
            }
        }
        if (e->value_type == JSONValueType_Undefined) {
            JSONFreeElement(e);
            e = ne;
        } else {
            JSONAddChildToElement(e, ne);
        }
        i++;
    } else {
        e = _JSONCreateElement();
        bool quoted = string[i] == '"';
    
        size_t j = i;
        while (_JSONCharIsValid(string, j, i, petc)) j++;
        if (j == i) return e;

        const size_t STRING_LENGTH = j - i - (1 * quoted);
        const size_t STRING_START = i + (1 * quoted);
        char * value_string = (char *) malloc(STRING_LENGTH + 1);
        if (!value_string) return NULL;
        strncpy(value_string, string + STRING_START, STRING_LENGTH - (1 * quoted));

        bool numeric = true;
        for (size_t n = 0; n < STRING_LENGTH && numeric; n++) numeric = isdigit(value_string[n]);
        if (numeric) {
            e->value_type = JSONValueType_Number;
            e->data.number = atof(value_string);
            j--;
        } else if (!quoted) {
            if (strcmp(value_string,"null") == 0) {
                e->value_type = JSONValueType_Null;
            } else if (strcmp(value_string,"true") == 0) {
                e->value_type = JSONValueType_Boolean;
                e->data.boolean = true;
            } else if (strcmp(value_string,"false") == 0) {
                e->value_type = JSONValueType_Boolean;
                e->data.boolean = false;
            }
            j--;
        } else {
            e->value_type = JSONValueType_String;
            e->data.string = value_string;
            while (isblank(string[j]) && string[j] != ':' && string[j] != ',') j++;
            switch (string[j]) {
                case ':' : {
                    j++;
                    JSONElement * v = _JSONGetElementFromString(string, &j, 0);
                    e->value_type = JSONValueType_NameValuePair;
                    char * name = e->data.string;
                    e->data.namevaluepair[1] = v;
                    e->data.namevaluepair[0] = name;
                    j--;
                }
            }
        }
        i = j + 1;
    }
    while ((isblank(string[i]) || string[i] == '\n')) i++;
    *offset = i;
    return e;
}

JSONElement * JSONReadElementFromFile(char * filename)
{
    if (!filename || filename[0] == 0) return NULL;

    FILE * infile = fopen(filename,"r");
    if (!infile) return NULL;

    struct stat st;
    stat(filename, &st);

    char * json_file_data = (char *) malloc(st.st_size + 1);
    size_t bytes_read = 0;
    if (json_file_data) {
        bytes_read = fread(json_file_data, 1, st.st_size, infile);
        if (bytes_read != st.st_size) {
            free(json_file_data);
            json_file_data = NULL;
        } else {
            json_file_data[st.st_size] = 0;
        }
    }
    fclose(infile);

    size_t offset = 0;
    return json_file_data ? _JSONGetElementFromString(json_file_data, &offset, 0) : NULL;
}

JSONElement ** JSONGetChildElementsArray(JSONElement * container_element, size_t * array_size_ptr)
{
    return JSONIsContainerElement(container_element) ? container_element->data.array : NULL;
}

size_t JSONGetChildElementsArraySize(JSONElement * container_element)
{
    return JSONIsContainerElement(container_element) ? container_element->length : 0;
}

double _JSONGetNumberFromString(char * string)
{
    bool numeric = true;
    for (size_t i = 0; i < JSON_MAX_STRING_VALUE_LENGTH && string[i] != 0 && numeric; i++) {
        numeric = isdigit(string[i]);
    }
    return numeric ? atof(string) : 0;
}

char * _JSONGetNumberAsString(double number, int dp)
{
    const int MAX_DOUBLE_CHAR_COUNT = 3 + DBL_MANT_DIG - DBL_MIN_EXP;
    char * num_buffer = (char *) malloc(MAX_DOUBLE_CHAR_COUNT + 1);
    if (!num_buffer) return NULL;
    
    char format_string[7];
    int fse;
    if (dp > 0) {
        fse = sprintf(format_string,"%%.%if",dp);
    } else if (dp == 0) {
        fse = sprintf(format_string,"%%li");
    } else {
        fse = sprintf(format_string,"%%f");
    }
    format_string[fse] = 0;

    int end_char = sprintf(num_buffer, format_string, number);
    num_buffer[end_char] = 0;
    return num_buffer;
}

double JSONGetValueAsDouble(JSONElement * element)
{
    if (!_JSONElementIsValid(element)) return 0;
    switch (element->value_type) {
        case JSONValueType_Null : return 0;
        case JSONValueType_Boolean : return element->data.boolean ? 1.0 : 0.0;
        case JSONValueType_Number : return element->data.number;
        case JSONValueType_NameValuePair : return JSONGetValueAsDouble((JSONElement *) element->data.namevaluepair[1]);
        case JSONValueType_String : return _JSONGetNumberFromString(element->data.string);
        default: return 0;
    }
}

long JSONGetValueAsLong(JSONElement * element)
{
    if (!_JSONElementIsValid(element)) return 0;
    switch (element->value_type) {
        case JSONValueType_Null : return 0;
        case JSONValueType_Boolean : return element->data.boolean ? 1 : 0;
        case JSONValueType_Number : return (long) element->data.number;
        case JSONValueType_NameValuePair : return (long) JSONGetValueAsLong((JSONElement *) element->data.namevaluepair[1]);
        case JSONValueType_String : return (long) _JSONGetNumberFromString(element->data.string);
        default: return 0;
    }
}

bool JSONGetValueAsBoolean(JSONElement * element)
{
    if (!_JSONElementIsValid(element)) return false;
    switch (element->value_type) {
        case JSONValueType_Null : return false;
        case JSONValueType_Boolean : return element->data.boolean;
        case JSONValueType_Number : return element->data.number != 0 ? true : false;
        case JSONValueType_NameValuePair : return JSONGetValueAsBoolean((JSONElement *) element->data.namevaluepair[1]);
        case JSONValueType_String : return _JSONGetNumberFromString(element->data.string) != 0 ? true : false;
        default: return false;
    }
}

char * JSONGetValueAsString(JSONElement * element, int dp)
{
    if (!_JSONElementIsValid(element)) return NULL;
    switch (element->value_type) {
        case JSONValueType_Null : return NULL;
        case JSONValueType_Boolean : return _JSONCopyString(element->data.boolean ? "true" : "false", NULL);
        case JSONValueType_Number : return _JSONGetNumberAsString(element->data.number, dp);
        case JSONValueType_NameValuePair : return JSONGetValueAsString((JSONElement *) element->data.namevaluepair[1], dp);
        case JSONValueType_String : return _JSONCopyString(element->data.string, NULL);
        default: return NULL;
    }
}

bool JSONSetValueUsingDouble(JSONElement * element, double value, int dp)
{
    if (!_JSONElementIsValid(element)) return false;
    switch (element->value_type) {
        case JSONValueType_Null : return true;
        case JSONValueType_Boolean : element->data.boolean = value != 0; return true;
        case JSONValueType_Number : element->data.number = value; return true;
        case JSONValueType_NameValuePair : return JSONSetValueUsingDouble((JSONElement *) element->data.namevaluepair[1], value, dp);
        case JSONValueType_String : { 
            free(element->data.string);
            element->data.string = _JSONGetNumberAsString(value, dp);
            return true;
        }
        default: return false;
    }
}

bool JSONSetValueUsingLong(JSONElement * element, long value)
{
    if (!_JSONElementIsValid(element)) return false;
    switch (element->value_type) {
        case JSONValueType_Null : return true;
        case JSONValueType_Boolean : element->data.boolean = value != 0; return true;
        case JSONValueType_Number : element->data.number = (double) value; return true;
        case JSONValueType_NameValuePair : return JSONSetValueUsingLong((JSONElement *) element->data.namevaluepair[1], value);
        case JSONValueType_String : { 
            free(element->data.string);
            element->data.string = _JSONGetNumberAsString(value, 0);
            return true;
        }
        default: return false;
    }
}

bool JSONSetValueUsingBoolean(JSONElement * element, bool value)
{
    if (!_JSONElementIsValid(element)) return false;
    switch (element->value_type) {
        case JSONValueType_Null : return true;
        case JSONValueType_Boolean : element->data.boolean = value;
        case JSONValueType_Number : element->data.number = value ? 1 : 0;
        case JSONValueType_NameValuePair : return JSONSetValueUsingBoolean((JSONElement *) element->data.namevaluepair[1], value);
        case JSONValueType_String : { 
            free(element->data.string);
            element->data.string = _JSONCopyString(value ? "true" : "false", NULL);
            return true;
        }
        default: return false;
    }
}

bool JSONSetValueUsingString(JSONElement * element, char * string)
{
    if (!_JSONElementIsValid(element)) return false;
    switch (element->value_type) {
        case JSONValueType_Null : return true;
        case JSONValueType_Boolean : element->data.boolean = string[0] != 0 ? true : false;
        case JSONValueType_Number : element->data.number = _JSONGetNumberFromString(string);
        case JSONValueType_NameValuePair : return JSONSetValueUsingString((JSONElement *) element->data.namevaluepair[1], string);
        case JSONValueType_String : { 
            free(element->data.string);
            element->data.string = _JSONCopyString(string, NULL);
            return true;
        }
        default: return false;
    }
}