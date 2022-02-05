///////////////////////////////////////////////////////////////////////////////
// NAME:            test-yaml-ser.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Yaml Serializer tests.
//
// CREATED:         02/04/2022
//
// LAST EDITED:     02/05/2022
//
// Copyright 2022, Ethan D. Twardy
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

#include <serdec/yaml.h>

#include <unity_fixture.h>

#include "my-struct.h"

TEST_GROUP(YamlSer);
TEST_SETUP(YamlSer) {}
TEST_TEAR_DOWN(YamlSer) {}

int my_struct_serialize_yaml(SerdecYamlSerializer* ser, const MyStruct* value)
{
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_start(ser));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_key(ser, "test"));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_bool(ser, value->test));

    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_key(ser, "a_number"));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_int(ser, value->a_number));

    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_key(ser, "a_string"));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_string(ser,
            value->a_string));

    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_key(ser,
            "list_of_four"));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_list_start(ser));
    for (size_t i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_int(ser,
                value->list_of_four[i]));
    }
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_list_end(ser));
    TEST_ASSERT_EQUAL_INT(0, serdec_yaml_serialize_map_end(ser));
    return 0;
}

static const char* BASIC_DOCUMENT = "\
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

TEST(YamlSer, BasicDocument) {
    MyStruct value = {
        .test = true,
        .a_number = 1,
        .a_string = "test",
        .list_of_four = {1, 2, 3, 4},
    };

    SerdecYamlSerializer* ser = serdec_yaml_serializer_new_string();
    int result = serdec_yaml_serialize_start(ser);
    TEST_ASSERT_EQUAL_INT(result, 0);
    result = my_struct_serialize_yaml(ser, &value);
    TEST_ASSERT_EQUAL_INT(result, 0);
    result = serdec_yaml_serialize_end(ser);
    TEST_ASSERT_EQUAL_INT(result, 0);

    const char* string = serdec_yaml_serializer_borrow_string(ser);
    TEST_ASSERT_NOT_NULL(string);
    TEST_ASSERT_EQUAL_STRING(string, BASIC_DOCUMENT);
    serdec_yaml_serializer_free(ser);
}

TEST_GROUP_RUNNER(YamlSer) {
    RUN_TEST_CASE(YamlSer, BasicDocument);
}

///////////////////////////////////////////////////////////////////////////////
