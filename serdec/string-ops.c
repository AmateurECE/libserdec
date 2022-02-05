///////////////////////////////////////////////////////////////////////////////
// NAME:            string-ops.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of string operations.
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

#include <stdlib.h>
#include <string.h>

#include <serdec/string-ops.h>

///////////////////////////////////////////////////////////////////////////////
// Public API
////

char* string_append_new_with_length(char* restrict string,
    const char* restrict buffer, size_t length)
{
    size_t new_length = strlen(string) + length + 1;
    char* result = malloc(new_length);
    memset(result, 0, new_length);
    strcat(result, string);
    strncat(result, buffer, length);
    free(string);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
