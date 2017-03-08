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
#include "dictobject.h"
#include <iostream>

PyObject* PyDict_Create(void) {
  PyDictObject* object = new PyDictObject;
  object->ob_refcnt = 1;
  object->ob_type = &PyDict_Type;

  return (PyObject*)object;
}

PyObject* PyDict_GetItem(PyObject* dict, PyObject* key) {
  std::size_t hash_value = key->ob_type->tp_hash(key);
  auto& table = ((PyDictObject*)dict)->ob_table;
  auto it = table.find(hash_value);
  if (it == table.end())
    return nullptr;
  return it->second;
}

void PyDict_SetItem(PyObject* dict, PyObject* key, PyObject* value) {
  std::size_t hash_value = key->ob_type->tp_hash(key);
  auto dict_object = (PyDictObject*)dict;
  dict_object->ob_table[hash_value] = value;
}

static void dict_print(PyObject* dict) {
  auto dict_object = (PyDictObject*)dict;
  std::cout << "{";
  for (auto& item : dict_object->ob_table) {
    std::cout << item.first << ": ";
    item.second->ob_type->tp_print(item.second);
    std::cout << ", ";
  }
  std::cout << "}" << std::endl;
}

PyObjectType PyDict_Type = {
  PyObject_HEAD_INIT(&PyType_Type),
  "dict",
  dict_print, // tp_print
  0,          // tp_add
  0,          // tp_hash
};
