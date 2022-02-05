///////////////////////////////////////////////////////////////////////////////
// NAME:            yaml.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the YAML deserializer.
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include <serdec/yaml.h>

// This struct maintains all internal state of the deserializer.
typedef struct SerdecYamlDeserializer {
    yaml_parser_t parser;
    yaml_event_t event;
} SerdecYamlDeserializer;

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

    bool done = false;
    while (!done) {
        if (!yaml_parser_parse(&deser->parser, &deser->event)) {
            free(deser);
            return NULL;
        }

        if (YAML_STREAM_START_EVENT != deser->event.type &&
            YAML_DOCUMENT_START_EVENT != deser->event.type) {
            break;
        }
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

    bool done = false;
    while (!done) {
        if (!yaml_parser_parse(&deser->parser, &deser->event)) {
            free(deser);
            return NULL;
        }

        if (YAML_STREAM_START_EVENT != deser->event.type &&
            YAML_DOCUMENT_START_EVENT != deser->event.type) {
            break;
        }
    }

    return deser;
}

// Free a de-serializer.
void serdec_yaml_deserializer_free(SerdecYamlDeserializer** deser) {
    if (NULL == *deser) {
        return;
    }

    free(*deser);
    *deser = NULL;
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
    if (YAML_STREAM_END_EVENT == deser->event.type ||
        YAML_DOCUMENT_END_EVENT == deser->event.type) {
        return 0;
    }

    if (YAML_MAPPING_START_EVENT != deser->event.type) {
        return -EINVAL;
    }

    yaml_event_delete(&deser->event);
    int result = -1;
    yaml_event_t key_event = {0};
    result = yaml_parser_parse(&deser->parser, &key_event);
    if (!result) {
        return -1 * result;
    }

    while (YAML_MAPPING_END_EVENT != key_event.type) {
        result = yaml_parser_parse(&deser->parser, &deser->event);
        if (YAML_SCALAR_EVENT != key_event.type || !result) {
            yaml_event_delete(&key_event);
            return -EINVAL;
        }

        result = callback(deser, user_data,
            (const char*)key_event.data.scalar.value);
        yaml_event_delete(&key_event);
        if (!result) {
            yaml_event_delete(&deser->event);
            yaml_parser_parse(&deser->parser, &deser->event);
        }
        memcpy(&key_event, &deser->event, sizeof(yaml_event_t));
    }

    yaml_event_delete(&deser->event);
    result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }
    return 1;
}

// Deserialize a list from the input stream. The callback is invoked for every
// list entry, and it's the responsibility of the callback to drive the
// deserializer to de-serialize interesting types from the input stream.
int serdec_yaml_deserialize_list(SerdecYamlDeserializer* deser,
    yaml_visit_list_callback* callback, void* user_data)
{
    if (YAML_STREAM_END_EVENT == deser->event.type ||
        YAML_DOCUMENT_END_EVENT == deser->event.type) {
        return 0;
    }

    if (YAML_SEQUENCE_START_EVENT != deser->event.type) {
        return -EINVAL;
    }

    yaml_event_delete(&deser->event);
    int result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }

    size_t index = 0;
    while (YAML_SEQUENCE_END_EVENT != deser->event.type) {
        result = callback(deser, user_data, index++);
        if (!result) {
            yaml_event_delete(&deser->event);
            result = yaml_parser_parse(&deser->parser, &deser->event);
            if (!result) {
                // TODO: Replace this with some negative value, since !result
                // implies that result == 0.
                return -1 * result;
            }
        }
    }

    yaml_event_delete(&deser->event);
    result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }

    return 1;
}

// De-serialize a boolean from the input stream. Return non-zero if parsing
// encountered an error, for any reason. This callback requires that booleans
// be either "true" or "false", and cannot be a value of "0" or non-zero.
int serdec_yaml_deserialize_bool(SerdecYamlDeserializer* deser, bool* value) {
    if (YAML_STREAM_END_EVENT == deser->event.type ||
        YAML_DOCUMENT_END_EVENT == deser->event.type) {
        return 0;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        return -EINVAL;
    }

    bool value_bool = false;
    if (!strcmp((const char*)deser->event.data.scalar.value, "true")) {
        value_bool = true;
    } else if (!strcmp((const char*)deser->event.data.scalar.value, "false")) {
        value_bool = false;
    } else {
        return -EINVAL;
    }

    // TODO: Some debug printing here would be nice.
    yaml_event_delete(&deser->event);
    int result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }

    *value = value_bool;
    return 1;
}

// De-serialize an integer value from the input stream. Return non-zero if
// parsing encountered an error, for any reason.
int serdec_yaml_deserialize_int(SerdecYamlDeserializer* deser, int* value) {
    if (YAML_STREAM_END_EVENT == deser->event.type ||
        YAML_DOCUMENT_END_EVENT == deser->event.type) {
        return 0;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        return -EINVAL;
    }

    char* end_pointer = NULL;
    int value_int = strtol((const char*)deser->event.data.scalar.value,
        &end_pointer, 10);
    if (NULL != end_pointer && 0 != *end_pointer) {
        return -EINVAL;
    }

    yaml_event_delete(&deser->event);
    int result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }

    *value = value_int;
    return 1;
}

// De-serialize a string value from the input stream. Return a number less than
// zero if parsing encounters an error.
int serdec_yaml_deserialize_string(SerdecYamlDeserializer* deser,
    char** value)
{
    if (YAML_STREAM_END_EVENT == deser->event.type ||
        YAML_DOCUMENT_END_EVENT == deser->event.type) {
        return 0;
    }

    if (YAML_SCALAR_EVENT != deser->event.type) {
        return -EINVAL;
    }

    // TODO: Might be able to avoid this allocation if we delete the previous
    // event at the beginning of the call, instead of at the end?
    *value = strdup((const char*)deser->event.data.scalar.value);
    yaml_event_delete(&deser->event);
    int result = yaml_parser_parse(&deser->parser, &deser->event);
    if (!result) {
        return -1 * result;
    }

    return 1;
}

///////////////////////////////////////////////////////////////////////////////
