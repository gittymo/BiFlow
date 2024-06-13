#include <stdlib.h>
#include <stdio.h>

#include "json.h"

int main(int argc, char ** argv)
{
    JSONElement * name = JSONCreateStringElement("Morgan Evans");
    JSONElement * age = JSONCreateNumberElement(47);

    JSONElement * nvpName = JSONCreateNameValuePairElement("Name", name);
    JSONElement * nvpAge = JSONCreateNameValuePairElement("Age", age);

    JSONElement * obj = JSONCreateObjectElement();
    JSONAddElementToObject(obj, nvpName);
    JSONAddElementToObject(obj, nvpAge);

    JSONElement * arr = JSONCreateArrayElement();
    JSONAddElementToArray(arr, name);
    JSONAddElementToArray(arr, obj);

    JSONOutputBuffer * ob = _JSONWriteElementToBuffer(arr, NULL, 0);

    _JSONWriteBufferToFile(ob, "test.json");

    JSONFreeElement(arr);

    exit(EXIT_SUCCESS);
}