///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml-ser.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Yaml serialization routines
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <yaml.h>

#include <serdec/yaml-error.h>
#include <serdec/yaml.h>
#include <serdec/string-ops.h>

static const char* SERDEC_YAML_ERROR_STRINGS[] = {
    [SERDEC_YAML_UNKNOWN_ERROR]="unknown error in libyaml",
    [SERDEC_YAML_WRONG_TYPE]="serializer is the wrong type for the operation",
};

static const int SERDEC_YAML_INDENT = 4;

typedef enum SerializerType {
    SERIALIZER_STRING,
} SerializerType;

typedef struct SerdecYamlSerializer {
    yaml_emitter_t emitter;
    yaml_event_t event;
    int error;
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
    return 1; // To indicate success
}

///////////////////////////////////////////////////////////////////////////////
// Error Handling
////

const char* serdec_yaml_serializer_strerror(SerdecYamlSerializer* ser) {
    if (0 > ser->error || SERDEC_YAML_MAX_ERROR <= ser->error) {
        return NULL;
    }

    switch (ser->error) {
    case SERDEC_YAML_SYSTEM_ERROR: return strerror(errno);
    case SERDEC_YAML_UNKNOWN_ERROR: return ser->emitter.problem;
    default:
        return SERDEC_YAML_ERROR_STRINGS[ser->error];
    }
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
    yaml_emitter_set_indent(&ser->emitter, SERDEC_YAML_INDENT);
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
        ser->error = SERDEC_YAML_WRONG_TYPE;
        return NULL;
    }
    return ser->serializer.serializer_data;
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
    yaml_stream_start_event_initialize(&ser->event, YAML_UTF8_ENCODING);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }

    yaml_version_directive_t version = {.major=1, .minor=1};
    yaml_document_start_event_initialize(&ser->event, &version, NULL, NULL, 0);
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

int serdec_yaml_serialize_end(SerdecYamlSerializer* ser) {
    yaml_document_end_event_initialize(&ser->event, 1);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }

    yaml_stream_end_event_initialize(&ser->event);
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

// Serialize a map to the output stream. To use this in an object serialization
// routine, first call _start(). For each map entry, call _key() with the key
// to associate with the desired value. Call one of the other serialization
// routines to serialize the value for the associated key into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_map_start(SerdecYamlSerializer* ser) {
    yaml_mapping_start_event_initialize(&ser->event, NULL,
        (const yaml_char_t*)YAML_MAP_TAG, 1, YAML_ANY_MAPPING_STYLE);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

int serdec_yaml_serialize_map_end(SerdecYamlSerializer* ser) {
    yaml_mapping_end_event_initialize(&ser->event);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

int serdec_yaml_serialize_map_key(SerdecYamlSerializer* ser, const char* key) {
    yaml_scalar_event_initialize(&ser->event, NULL, (yaml_char_t*)YAML_STR_TAG,
        (const yaml_char_t*)key, strlen(key), 1, 0, YAML_PLAIN_SCALAR_STYLE);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

// Serialize a list to the output stream. To use this in an object
// serialization routine, first call start. For each element in the list, call
// a serialization routine to serialize a single element into the output
// stream. Finally, call _end().
int serdec_yaml_serialize_list_start(SerdecYamlSerializer* ser) {
    yaml_sequence_start_event_initialize(&ser->event, NULL, NULL, 0,
        YAML_ANY_SEQUENCE_STYLE);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

int serdec_yaml_serialize_list_end(SerdecYamlSerializer* ser) {
    yaml_sequence_end_event_initialize(&ser->event);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

// Serialize a boolean to the output stream. Return non-zero if parsing
// encountered an error, for any reason.
int serdec_yaml_serialize_bool(SerdecYamlSerializer* ser, bool value) {
    const char* string_value = "false";
    if (value) {
        string_value = "true";
    }
    yaml_scalar_event_initialize(&ser->event, NULL,
        (const yaml_char_t*)YAML_STR_TAG, (const yaml_char_t*)string_value,
        strlen(string_value), 1, 0, YAML_PLAIN_SCALAR_STYLE);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

// Serialize an integer value to the output stream. Return non-zero if parsing
// encountered an error, for any reason.
int serdec_yaml_serialize_int(SerdecYamlSerializer* ser, int value) {
    char buffer[64] = {0}; // May as well use a whole cache line?
    int result = snprintf(buffer, sizeof(buffer), "%d", value);
    if (0 > result) {
        ser->error = SERDEC_YAML_SYSTEM_ERROR;
        return result;
    }

    yaml_scalar_event_initialize(&ser->event, NULL,
        (const yaml_char_t*)YAML_INT_TAG, (const yaml_char_t*)buffer, result,
        1, 0, YAML_PLAIN_SCALAR_STYLE);
    result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

// Serialize a string value to the output stream. Return non-zero if parsing
// encounters an error.
int serdec_yaml_serialize_string(SerdecYamlSerializer* ser, const char* value)
{
    // TODO: Could use "YAML_LITERAL_SCALAR_STYLE" in here to get '|' for long
    // strings.
    yaml_scalar_event_initialize(&ser->event, NULL,
        (const yaml_char_t*)YAML_STR_TAG, (const yaml_char_t*)value,
        strlen(value), 1, 1, YAML_SINGLE_QUOTED_SCALAR_STYLE);
    int result = 0;
    if (!yaml_emitter_emit(&ser->emitter, &ser->event)) {
        ser->error = SERDEC_YAML_UNKNOWN_ERROR;
        result = ser->error;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
