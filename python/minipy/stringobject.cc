// Copyright (c) 2017 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include "stringobject.h"
#include <cstdio>
#include <cstring>
#include <iostream>

static void string_print(PyObject* object) {
  PyStringObject* str_object = (PyStringObject*)object;
  std::cout << str_object->ob_svalue << std::endl;
}

static PyObject* string_add(PyObject* lobject, PyObject* robject) {
  PyStringObject* lstr = (PyStringObject*)lobject;
  PyStringObject* rstr = (PyStringObject*)robject;
  PyStringObject* result = (PyStringObject*)PyString_Create(nullptr);
  if (result == nullptr) {
    std::cerr << "have no enough memory !!!" << std::endl;
    exit(1);
  }
  else {
    std::snprintf(result->ob_svalue, sizeof(result->ob_svalue),
        "%s%s", lstr->ob_svalue, rstr->ob_svalue);
  }

  return (PyObject*)result;
}

static long string_hash(PyObject* object) {
  PyStringObject* str_object = (PyStringObject*)object;
  if (str_object->ob_shash != -1)
    return str_object->ob_shash;

  Py_ssize_t size = str_object->ob_ssize;
  std::uint8_t* p = (std::uint8_t*)str_object->ob_svalue;
  long hash = *p << 7;
  while (--size >= 0)
    hash = (1000003 * hash) ^ *p++;
  if (hash == -1)
    hash = -2;
  str_object->ob_shash = hash;

  return hash;
}

PyObjectType PyString_Type = {
  PyObject_HEAD_INIT(&PyType_Type),
  "str",
  string_print, // tp_print
  string_add,   // tp_add
  string_hash,  // tp_hash
};

PyObject* PyString_Create(const char* s) {
  PyStringObject* object = new PyStringObject;
  object->ob_refcnt = 1;
  object->ob_type = &PyString_Type;
  object->ob_ssize = (s == nullptr) ? 0 : (Py_ssize_t)std::strlen(s);
  object->ob_shash = -1;
  std::memset(object->ob_svalue, 0, sizeof(object->ob_svalue));
  if (s != nullptr)
    std::strcpy(object->ob_svalue, s);

  return (PyObject*)object;
}
