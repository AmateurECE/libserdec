///////////////////////////////////////////////////////////////////////////////
// NAME:            error.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Error types for codecs
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

#ifndef SERDEC_YAML_ERROR_H
#define SERDEC_YAML_ERROR_H

enum {
    // YAML Serializer errors
    SERDEC_YAML_NO_ERROR,
    SERDEC_YAML_UNKNOWN_ERROR,
    SERDEC_YAML_SYSTEM_ERROR,
    SERDEC_YAML_WRONG_TYPE,
    SERDEC_YAML_MAX_ERROR,
    SERDEC_YAML_UNEXPECTED_EVENT,
    SERDEC_YAML_INVALID_BOOLEAN_TOKEN,
    SERDEC_YAML_CALLBACK_SIGNALED_ERROR,
};

#endif // SERDEC_YAML_ERROR_H

///////////////////////////////////////////////////////////////////////////////
