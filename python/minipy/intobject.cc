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
#include "intobject.h"
#include <cstdio>
#include <iostream>

static void int_print(PyObject* object) {
  PyIntObject* int_object = (PyIntObject*)object;
  std::cout << int_object->ob_ivalue << std::endl;
}

static PyObject* int_add(PyObject* lobject, PyObject* robject) {
  PyIntObject* lint = (PyIntObject*)lobject;
  PyIntObject* rint = (PyIntObject*)robject;
  PyIntObject* result = (PyIntObject*)PyInt_Create(0);
  if (result == nullptr) {
    std::cerr << "have no enough memory !!!" << std::endl;
    exit(1);
  }
  else {
    result->ob_ivalue = lint->ob_ivalue + rint->ob_ivalue;
  }

  return (PyObject*)result;
}

static long int_hash(PyObject* object) {
  return (std::size_t)((PyIntObject*)object)->ob_ivalue;
}

PyObjectType PyInt_Type = {
  PyObject_HEAD_INIT(&PyType_Type),
  "int",
  int_print,  // tp_print
  int_add,    // tp_add
  int_hash,   // tp_hash
};

PyObject* PyInt_Create(int value) {
  PyIntObject* object = new PyIntObject;
  object->ob_refcnt = 1;
  object->ob_type = &PyInt_Type;
  object->ob_ivalue = value;

  return (PyObject*)object;
}
