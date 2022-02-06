///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the YAML deserializer.
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include <serdec/yaml.h>
#include <serdec/yaml-error.h>

static const char* SERDEC_YAML_ERROR_STRINGS[] = {
    [SERDEC_YAML_UNKNOWN_ERROR]="unknown error in libyaml",
    [SERDEC_YAML_WRONG_TYPE]="serializer is the wrong type for the operation",
    [SERDEC_YAML_UNEXPECTED_EVENT]="expected a different event in the stream",
    [SERDEC_YAML_INVALID_BOOLEAN_TOKEN]="expected either 'true' or 'false'",
    [SERDEC_YAML_CALLBACK_SIGNALED_ERROR]="callback returned non-zero",
};

// This struct maintains all internal state of the deserializer.
typedef struct SerdecYamlDeserializer {
    yaml_parser_t parser;
    yaml_event_t event;
    yaml_event_t event_buffer;
    int error;
} SerdecYamlDeserializer;

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static int yaml_peek_event(SerdecYamlDeserializer* deser) {
    if (YAML_NO_EVENT != deser->event_buffer.type) {
        yaml_event_delete(&deser->event_buffer);
    }

    if (!yaml_parser_parse(&deser->parser, &deser->event_buffer)) {
        deser->error = SERDEC_YAML_UNKNOWN_ERROR;
        return deser->error;
    }
    return 0;
}

static int yaml_next_event(SerdecYamlDeserializer* deser) {
    yaml_event_delete(&deser->event);
    if (YAML_NO_EVENT == deser->event_buffer.type) {
        if (!yaml_parser_parse(&deser->parser, &deser->event)) {
            deser->error = SERDEC_YAML_UNKNOWN_ERROR;
            return deser->error;
        }
    } else {
        memcpy(&deser->event, &deser->event_buffer, sizeof(yaml_event_t));
        memset(&deser->event_buffer, 0, sizeof(yaml_event_t));
    }

    return 0;
}

static int prepare_deserializer(SerdecYamlDeserializer* deser) {
    bool done = false;
    while (!done) {
        if (yaml_peek_event(deser)) {
            return deser->error;
        }

        if (YAML_STREAM_START_EVENT == deser->event_buffer.type ||
            YAML_DOCUMENT_START_EVENT == deser->event_buffer.type) {
            if (yaml_next_event(deser)) {
                return deser->error;
            }
        } else {
            done = true;
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Error Handling
////

const char* serdec_yaml_deserializer_strerror(SerdecYamlDeserializer* deser) {
    if (0 > deser->error || SERDEC_YAML_MAX_ERROR <= deser->error) {
        return NULL;
    }

    switch (deser->error) {
    case SERDEC_YAML_SYSTEM_ERROR: return strerror(errno);
    case SERDEC_YAML_UNKNOWN_ERROR: return deser->parser.problem;
    default:
        return SERDEC_YAML_ERROR_STRINGS[deser->error];
    }
}

///////////////////////////////////////////////////////////////////////////////
// De-serializer Initialization
////

// Initialize a de-serializer from the given input string.
SerdecYamlDeserializer* serdec_yaml_deserializer_new_string(const char* string,
    size_t string_length)
{
    SerdecYamlDeserializer* deser = malloc(sizeof(SerdecYamlDeserializer));
    if (NULL == deser) {
        return NULL;
    }

    memset(deser, 0, sizeof(SerdecYamlDeserializer));
    yaml_parser_initialize(&deser->parser);
    yaml_parser_set_input_string(&deser->parser, (const unsigned char*)string,
        string_length);

    if (prepare_deserializer(deser)) {
        yaml_parser_delete(&deser->parser);
        free(deser);
        return NULL;
    }

    return deser;
}

// Create a YAML deserializer from the input file.
SerdecYamlDeserializer* serdec_yaml_deserializer_new_file(FILE* input_file) {
    SerdecYamlDeserializer* deser = malloc(sizeof(SerdecYamlDeserializer));
    if (NULL == deser) {
        return NULL;
    }

    memset(deser, 0, sizeof(SerdecYamlDeserializer));
    yaml_parser_initialize(&deser->parser);
    yaml_parser_set_input_file(&deser->parser, input_file);

    if (prepare_deserializer(deser)) {
        yaml_parser_delete(&deser->parser);
        free(deser);
        return NULL;
    }

    return deser;
}

// Free a de-serializer.
void serdec_yaml_deserializer_free(SerdecYamlDeserializer* deser) {
    yaml_event_delete(&deser->event);
    yaml_event_delete(&deser->event_buffer);
    yaml_parser_delete(&deser->parser);
    free(deser);
}

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
    yaml_visit_map_callback* callback, void* user_data)
{
    if (yaml_next_event(deser)) {
        return deser->error;
    }

    if (YAML_MAPPING_START_EVENT != deser->event.type) {
        deser->error = SERDEC_YAML_UNEXPECTED_EVENT;
        return deser->error;
    }

    yaml_event_t key_event = {0};
    int result = 0;
    while (!(result = yaml_peek_event(deser)) &&
        YAML_MAPPING_END_EVENT != deser->event_buffer.type)
    {
        // Event in event_buffer is key event
        memcpy(&key_event, &deser->event_buffer, sizeof(deser->event_buffer));
        memset(&deser->event_buffer, 0, sizeof(deser->event_buffer));
        result = callback(deser, user_data,
            (const char*)key_event.data.scalar.value);
        yaml_event_delete(&key_event);
        if (result) {
            deser->error = SERDEC_YAML_CALLBACK_SIGNALED_ERROR;
            return deser->error;
        }
    }

    yaml_event_delete(&deser->event_buffer);
    return 0;
}

// Deserialize a list from the input stream. The callback is invoked for every
// list entry, and it's the responsibility of the callback to drive the
// deserializer to de-serialize interesting types from the input stream.
int serdec_yaml_deserialize_list(SerdecYamlDeserializer* deser,
    yaml_visit_list_callback* callback, void* user_data)
{
    if (yaml_next_event(deser)) {
        return deser->error;
    }

    if (YAML_SEQUENCE_START_EVENT != deser->event.type) {
        deser->error = SERDEC_YAML_UNEXPECTED_EVENT;
        return deser->error;
    }

    size_t index = 0;
    int result = 0;
    while (!(result = yaml_peek_event(deser)) &&
        YAML_SEQUENCE_END_EVENT != deser->event_buffer.type)
    {
        if (callback(deser, user_data, index++)) {
            deser->error = SERDEC_YAML_CALLBACK_SIGNALED_ERROR;
            return deser->error;
        }
    }

    yaml_event_delete(&deser->event_buffer);
    return result;
}

// De-serialize a boolean from the input stream. Return non-zero if parsing
// encountered an error, for any reason. This callback requires that booleans
// be either "true" or "false", and cannot be a value of "0" or non-zero.
int serdec_yaml_deserialize_bool(SerdecYamlDeserializer* deser, bool* value) {
    if (yaml_next_event(deser)) {
        return deser->error;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        deser->error = SERDEC_YAML_UNEXPECTED_EVENT;
        return deser->error;
    }

    bool value_bool = false;
    if (!strcmp((const char*)deser->event.data.scalar.value, "true")) {
        value_bool = true;
    } else if (!strcmp((const char*)deser->event.data.scalar.value, "false")) {
        value_bool = false;
    } else {
        deser->error = SERDEC_YAML_INVALID_BOOLEAN_TOKEN;
        return deser->error;
    }

    *value = value_bool;
    return 0;
}

// De-serialize an integer value from the input stream. Return non-zero if
// parsing encountered an error, for any reason.
int serdec_yaml_deserialize_int(SerdecYamlDeserializer* deser, int* value) {
    if (yaml_next_event(deser)) {
        return deser->error;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        deser->error = SERDEC_YAML_UNEXPECTED_EVENT;
        return deser->error;
    }

    char* end_pointer = NULL;
    int value_int = strtol((const char*)deser->event.data.scalar.value,
        &end_pointer, 10);
    if (NULL != end_pointer && 0 != *end_pointer) {
        deser->error = SERDEC_YAML_SYSTEM_ERROR;
        return deser->error;
    }

    *value = value_int;
    return 0;
}

// De-serialize a string value from the input stream. Return non-zero if
// parsing encounters an error.
int serdec_yaml_deserialize_string(SerdecYamlDeserializer* deser,
    const char** value)
{
    if (yaml_next_event(deser)) {
        return deser->error;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        deser->error = SERDEC_YAML_UNEXPECTED_EVENT;
        return deser->error;
    }

    *value = (const char*)deser->event.data.scalar.value;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
