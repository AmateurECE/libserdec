///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml-ser.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Yaml serialization routines
//
// CREATED:         02/04/2022
//
// LAST EDITED:     02/04/2022
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

#include <assert.h>
#include <stdlib.h>

#include <yaml.h>

#include <serdec/yaml-error.h>
#include <serdec/yaml.h>
#include <serdec/string-ops.h>

static const char* SERR_YAML_ERROR_STRINGS[] = {
    [SERR_YAML_WRONG_TYPE]="serializer is the wrong type for the operation",
};

typedef enum SerializerType {
    SERIALIZER_STRING,
} SerializerType;

typedef struct SerdecYamlSerializer {
    yaml_emitter_t emitter;
    yaml_event_t event;
    int errno;
    struct {
        SerializerType type;
        void (*free)(void* serializer_data);
        void* serializer_data;
    } serializer;
} SerdecYamlSerializer;

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static int string_write(void* user_data, unsigned char* buffer, size_t length)
{
    SerdecYamlSerializer* ser = (SerdecYamlSerializer*)user_data;
    char** string = (char**)&(ser->serializer.serializer_data);
    *string = string_append_new_with_length(*string, (const char*)buffer,
        length);
    return length;
}

///////////////////////////////////////////////////////////////////////////////
// Error Handling
////

const char* serdec_yaml_serializer_strerror(SerdecYamlSerializer* ser) {
    if (ser->errno >= 0 && ser->errno < SERR_YAML_MAX_ERROR) {
        return SERR_YAML_ERROR_STRINGS[ser->errno];
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Serializer Initialization
////

// Initialize a serializer from the given input string.
SerdecYamlSerializer* serdec_yaml_serializer_new_buffer(char* buffer,
    size_t buffer_length);

// Initialize a serializer which will generate a string. The string will be
// allocated with malloc(3), and can be borrowed with _borrow_string().
SerdecYamlSerializer* serdec_yaml_serializer_new_string() {
    SerdecYamlSerializer* ser = malloc(sizeof(SerdecYamlSerializer));
    assert(NULL != ser);
    memset(ser, 0, sizeof(*ser));
    yaml_emitter_initialize(&ser->emitter);
    yaml_emitter_set_output(&ser->emitter, string_write, ser);

    char* string = malloc(sizeof(char));
    assert(NULL != string);
    *string = '\0';
    ser->serializer.serializer_data = string;

    ser->serializer.free = free;
    ser->serializer.type = SERIALIZER_STRING;
    return ser;
}

const char* serdec_yaml_serializer_borrow_string(SerdecYamlSerializer* ser)
{
    if (SERIALIZER_STRING != ser->serializer.type) {
        ser->errno = SERR_YAML_WRONG_TYPE;
        return NULL;
    }
    return *(char**)ser->serializer.serializer_data;
}

// Initialize a serializer from the given input string.
SerdecYamlSerializer* serdec_yaml_serializer_new_file(FILE* input_file);

// Free a serializer.
void serdec_yaml_serializer_free(SerdecYamlSerializer* ser) {
    ser->serializer.free(ser->serializer.serializer_data);
    yaml_emitter_delete(&ser->emitter);
    free(ser);
}

///////////////////////////////////////////////////////////////////////////////
// Serializer Routines
////

// The serializer framework requires calling _start() before beginning to
// serialize any data structures. Likewise, it's necessary to call _end()
// before extracting any output from the serializer, or else the stream may
// not be terminated.
int serdec_yaml_serialize_start(SerdecYamlSerializer* ser) {
    return 0;
}

int serdec_yaml_serialize_end(SerdecYamlSerializer* ser) {
    return 0;
}

// Serialize a map to the output stream. To use this in an object serialization
// routine, first call _start(). For each map entry, call _key() with the key
// to associate with the desired value. Call one of the other serialization
// routines to serialize the value for the associated key into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_map_start(SerdecYamlSerializer* ser) {
    return 0;
}

int serdec_yaml_serialize_map_end(SerdecYamlSerializer* ser) {
    return 0;
}

int serdec_yaml_serialize_map_key(SerdecYamlSerializer* ser, const char* key) {
    return 0;
}

// Serialize a list to the output stream. To use this in an object
// serialization routine, first call start. For each element in the list, call
// a serialization routine to serialize a single element into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_list_start(SerdecYamlSerializer* ser) {
    return 0;
}

int serdec_yaml_serialize_list_end(SerdecYamlSerializer* ser) {
    return 0;
}

// Serialize a boolean to the output stream. Return a number less than zero
// if parsing encountered an error, for any reason.
int serdec_yaml_serialize_bool(SerdecYamlSerializer* ser, bool value) {
    return 0;
}

// Serialize an integer value to the output stream. Return a number less than
// zero if parsing encountered an error, for any reason.
int serdec_yaml_serialize_int(SerdecYamlSerializer* ser, int value) {
    return 0;
}

// Serialize a string value to the output stream. Return a number less than
// zero if parsing encounters an error.
int serdec_yaml_serialize_string(SerdecYamlSerializer* ser, const char* value)
{ return 0; }

///////////////////////////////////////////////////////////////////////////////
