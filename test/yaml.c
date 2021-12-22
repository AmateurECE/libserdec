///////////////////////////////////////////////////////////////////////////////
// NAME:            example.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Example script
//
// CREATED:         12/20/2021
//
// LAST EDITED:     12/21/2021
//
// Copyright 2021, Ethan D. Twardy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
////

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <gobiserde/yaml.h>

typedef struct MyStruct {
    bool test;
    int a_number;
    char* a_string;
    int list[4];
} MyStruct;

int my_struct_visit_list_entry(yaml_deserializer* deser, void* user_data,
    size_t index)
{
    MyStruct* object = (MyStruct*)user_data;
    return gobiserde_yaml_deserialize_int(deser, &object->list[index]);
}

int my_struct_visit_map_entry(yaml_deserializer* deser, void* user_data,
    const char* key)
{
    MyStruct* object = (MyStruct*)user_data;
    int result = 0;
    if (!strcmp(key, "test")) {
        result = gobiserde_yaml_deserialize_bool(deser, &object->test);
    } else if (!strcmp(key, "a_number")) {
        result = gobiserde_yaml_deserialize_int(deser, &object->a_number);
    } else if (!strcmp(key, "list_of_four")) {
        result = gobiserde_yaml_deserialize_list(deser,
            my_struct_visit_list_entry, object);
    } else if (!strcmp(key, "a_string")) {
        result = gobiserde_yaml_deserialize_string(deser, &object->a_string);
    }

    if (!result) {
        return -EINVAL;
    }

    return result;
}

int my_struct_deserialize_yaml(yaml_deserializer* deser, MyStruct* value)
{
    return gobiserde_yaml_deserialize_map(deser, my_struct_visit_map_entry,
        value);
}

const char* DOCUMENT = "\
test: true\n\
a_number: 1\n\
a_string: 'test'\n\
list_of_four:\n\
    - 1\n\
    - 2\n\
    - 3\n\
    - 4\n\
";

int main() {
    yaml_deserializer* deser = gobiserde_yaml_deserializer_new_string(DOCUMENT,
        strlen(DOCUMENT));
    assert(NULL != deser);
    MyStruct my_struct = {0};
    int result = my_struct_deserialize_yaml(deser, &my_struct);
    assert(1 == result);
    assert(true == my_struct.test);
    assert(1 == my_struct.a_number);
    assert(1 == my_struct.list[0]);
    assert(2 == my_struct.list[1]);
    assert(3 == my_struct.list[2]);
    assert(4 == my_struct.list[3]);
    assert(!strcmp(my_struct.a_string, "test"));
}

///////////////////////////////////////////////////////////////////////////////
