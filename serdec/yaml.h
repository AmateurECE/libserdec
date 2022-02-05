///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Yaml serialization/deserialization
//
// CREATED:         12/20/2021
//
// LAST EDITED:     02/04/2022
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

#ifndef SERDEC_YAML_H
#define SERDEC_YAML_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// This struct maintains all internal state of the deserializer.
typedef struct SerdecYamlDeserializer SerdecYamlDeserializer;
typedef struct SerdecYamlSerializer SerdecYamlSerializer;

///////////////////////////////////////////////////////////////////////////////
// Error Handling
////

// Obtain error strings from the de/serializers.
const char* serdec_yaml_deserializer_strerror(SerdecYamlDeserializer* deser);
const char* serdec_yaml_serializer_strerror(SerdecYamlSerializer* ser);

///////////////////////////////////////////////////////////////////////////////
// De-serializer Initialization
////

// Initialize a de-serializer from the given input string.
SerdecYamlDeserializer* serdec_yaml_deserializer_new_string(const char* string,
    size_t string_length);

// Initialize a de-serializer from the given input string.
SerdecYamlDeserializer* serdec_yaml_deserializer_new_file(FILE* input_file);

// Free a de-serializer.
void serdec_yaml_deserializer_free(SerdecYamlDeserializer** deser);

///////////////////////////////////////////////////////////////////////////////
// De-serialization Routines
////

// This callback is to "visit" (i.e. handle) entries of a map. This is a
// user-defined callback.
typedef int yaml_visit_map_callback(SerdecYamlDeserializer* deser,
    void* user_data, const char* key);

// De-serialize a map from the input stream. Return non-zero if parsing
// encountered an error, for any reason.
int serdec_yaml_deserialize_map(SerdecYamlDeserializer* deser,
    yaml_visit_map_callback* callback, void* user_data);

// This callback is invoked to "visit" (i.e. handle) entries of a list. This is
// a user-defined callback. <index> is n - 1, where n is the number of times
// the callback has been invoked.
typedef int yaml_visit_list_callback(SerdecYamlDeserializer* deser,
    void* user_data, size_t index);

// Deserialize a list from the input stream. The callback is invoked for every
// list entry, and it's the responsibility of the callback to drive the
// deserializer to de-serialize interesting types from the input stream.
int serdec_yaml_deserialize_list(SerdecYamlDeserializer* deser,
    yaml_visit_list_callback* callback, void* user_data);

// De-serialize a boolean from the input stream. Return a number less than zero
// if parsing encountered an error, for any reason. This callback requires that
// booleans be either "true" or "false", and cannot be a value of "0" or
// non-zero.
int serdec_yaml_deserialize_bool(SerdecYamlDeserializer* deser, bool* value);

// De-serialize an integer value from the input stream. Return a number less
// than zero if parsing encountered an error, for any reason.
int serdec_yaml_deserialize_int(SerdecYamlDeserializer* deser, int* value);

// De-serialize a string value from the input stream. Return a number less than
// zero if parsing encounters an error.
int serdec_yaml_deserialize_string(SerdecYamlDeserializer* deser,
    char** value);

///////////////////////////////////////////////////////////////////////////////
// Serializer Initialization
////

// Initialize a serializer which writes to the buffer. If the buffer does not
// contain enough space, an error is signaled.
SerdecYamlSerializer* serdec_yaml_serializer_new_buffer(char* buffer,
    size_t buffer_length);

// Initialize a serializer which will generate a string. The string will be
// allocated with malloc(3), and can be borrowed with _borrow_string().
SerdecYamlSerializer* serdec_yaml_serializer_new_string();
const char* serdec_yaml_serializer_borrow_string(SerdecYamlSerializer* ser);

// Initialize a serializer from the given input string.
SerdecYamlSerializer* serdec_yaml_serializer_new_file(FILE* input_file);

// Free a serializer.
void serdec_yaml_serializer_free(SerdecYamlSerializer* ser);

///////////////////////////////////////////////////////////////////////////////
// Serializer Routines
////

// The serializer framework requires calling _start() before beginning to
// serialize any data structures. Likewise, it's necessary to call _end()
// before extracting any output from the serializer, or else the stream may
// not be terminated.
int serdec_yaml_serialize_start(SerdecYamlSerializer* ser);
int serdec_yaml_serialize_end(SerdecYamlSerializer* ser);

// Serialize a map to the output stream. To use this in an object serialization
// routine, first call _start(). For each map entry, call _key() with the key
// to associate with the desired value. Call one of the other serialization
// routines to serialize the value for the associated key into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_map_start(SerdecYamlSerializer* ser);
int serdec_yaml_serialize_map_end(SerdecYamlSerializer* ser);
int serdec_yaml_serialize_map_key(SerdecYamlSerializer* ser, const char* key);

// Serialize a list to the output stream. To use this in an object
// serialization routine, first call start. For each element in the list, call
// a serialization routine to serialize a single element into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_list_start(SerdecYamlSerializer* ser);
int serdec_yaml_serialize_list_end(SerdecYamlSerializer* ser);

// Serialize a boolean to the output stream. Return a number less than zero
// if parsing encountered an error, for any reason.
int serdec_yaml_serialize_bool(SerdecYamlSerializer* ser, bool value);

// Serialize an integer value to the output stream. Return a number less than
// zero if parsing encountered an error, for any reason.
int serdec_yaml_serialize_int(SerdecYamlSerializer* ser, int value);

// Serialize a string value to the output stream. Return a number less than
// zero if parsing encounters an error.
int serdec_yaml_serialize_string(SerdecYamlSerializer* ser, const char* value);

#endif // SERDEC_YAML_H

///////////////////////////////////////////////////////////////////////////////
