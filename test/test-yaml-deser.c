///////////////////////////////////////////////////////////////////////////////
// NAME:            example.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Example script
//
// CREATED:         12/20/2021
//
// LAST EDITED:     02/06/2022
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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <serdec/yaml.h>

#include <unity_fixture.h>

#include "my-struct.h"

TEST_GROUP(YamlDeser);
TEST_SETUP(YamlDeser) {}
TEST_TEAR_DOWN(YamlDeser) {}

int my_struct_visit_list_entry(SerdecYamlDeserializer* deser, void* user_data,
    size_t index)
{
    MyStruct* object = (MyStruct*)user_data;
    TEST_ASSERT(0 == serdec_yaml_deserialize_int(deser,
            &object->list_of_four[index]));
    return 0;
}

int my_struct_visit_map_entry(SerdecYamlDeserializer* deser, void* user_data,
    const char* key)
{
    MyStruct* object = (MyStruct*)user_data;
    if (!strcmp(key, "test")) {
        TEST_ASSERT(0 == serdec_yaml_deserialize_bool(deser, &object->test));
    } else if (!strcmp(key, "a_number")) {
        TEST_ASSERT(0 == serdec_yaml_deserialize_int(deser,
                &object->a_number));
    } else if (!strcmp(key, "list_of_four")) {
        TEST_ASSERT(0 == serdec_yaml_deserialize_list(deser,
                my_struct_visit_list_entry, object));
    } else if (!strcmp(key, "a_string")) {
        const char* temp = NULL;
        TEST_ASSERT(0 == serdec_yaml_deserialize_string(deser, &temp));
        // I would normally use strdup here, but apparently unity doesn't play
        // well with memory alloc'd by strdup.
        size_t length = strlen(temp) + 1;
        object->a_string = malloc(length);
        memset(object->a_string, 0, length);
        strcpy(object->a_string, temp);
    }

    return 0;
}

int my_struct_deserialize_yaml(SerdecYamlDeserializer* de, MyStruct* value)
{ return serdec_yaml_deserialize_map(de, my_struct_visit_map_entry, value); }

const char* DOCUMENT = "\
%YAML 1.1\n\
---\n\
test: true\n\
a_number: 1\n\
a_string: 'test'\n\
list_of_four:\n\
    - 1\n\
    - 2\n\
    - 3\n\
    - 4\n\
";

TEST(YamlDeser, BasicDocument) {
    SerdecYamlDeserializer* deser = serdec_yaml_deserializer_new_string(
        DOCUMENT, strlen(DOCUMENT));
    TEST_ASSERT(NULL != deser);
    MyStruct my_struct = {0};
    TEST_ASSERT(0 == my_struct_deserialize_yaml(deser, &my_struct));
    TEST_ASSERT(true == my_struct.test);
    TEST_ASSERT(1 == my_struct.a_number);
    TEST_ASSERT(1 == my_struct.list_of_four[0]);
    TEST_ASSERT(2 == my_struct.list_of_four[1]);
    TEST_ASSERT(3 == my_struct.list_of_four[2]);
    TEST_ASSERT(4 == my_struct.list_of_four[3]);
    TEST_ASSERT(!strcmp(my_struct.a_string, "test"));
    free(my_struct.a_string);
    serdec_yaml_deserializer_free(deser);
}

TEST_GROUP_RUNNER(YamlDeser) {
    RUN_TEST_CASE(YamlDeser, BasicDocument);
}

///////////////////////////////////////////////////////////////////////////////
