///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Yaml serialization/deserialization
//
// CREATED:         12/20/2021
//
// LAST EDITED:     12/20/2021
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

#ifndef GOBISERDE_YAML_H
#define GOBISERDE_YAML_H

#include <stdbool.h>
#include <stddef.h>

// This struct maintains all internal state of the deserializer.
typedef struct yaml_deserializer yaml_deserializer;

///////////////////////////////////////////////////////////////////////////////
// De-serializer initialization
////

// Initialize a de-serializer from the given input string.
yaml_deserializer* gobiserde_yaml_deserializer_new_string(const char* string,
    size_t string_length);

// Free a de-serializer.
void gobiserde_yaml_deserializer_free(yaml_deserializer** deser);

///////////////////////////////////////////////////////////////////////////////
// De-serialization Routines
////

// This callback is to "visit" (i.e. handle) entries of a map. This is a
// user-defined callback.
typedef int yaml_visit_map_callback(yaml_deserializer* deser, void* user_data,
    const char* key);

// De-serialize a map from the input stream. Return non-zero if parsing
// encountered an error, for any reason.
int gobiserde_yaml_deserialize_map(yaml_deserializer* deser,
    yaml_visit_map_callback* callback, void* user_data);

// De-serialize a boolean from the input stream. Return non-zero if parsing
// encountered an error, for any reason. This callback requires that booleans
// be either "true" or "false", and cannot be a value of "0" or non-zero.
int gobiserde_yaml_deserialize_bool(yaml_deserializer* deser, bool* value);

// De-serialize an integer value from the input stream. Return non-zero if
// parsing encountered an error, for any reason.
int gobiserde_yaml_deserialize_int(yaml_deserializer* deser, int* value);

#endif // GOBISERDE_YAML_H

///////////////////////////////////////////////////////////////////////////////
