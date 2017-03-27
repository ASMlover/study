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
#include <cctype>
#include <iostream>
#include "minipy.h"
#include "dictobject.h"
#include "intobject.h"
#include "stringobject.h"

static PyObject* g_ENV = (PyObject*)PyDict_Create();

static bool _Py_IsDigit(const std::string& s) {
  std::size_t n = s.size();
  for (std::size_t i = 0; i < n; ++i) {
    if (!std::isdigit(s[i]))
      return false;
  }
  return true;
}

static PyObject*
Py_GetObjectBySymbol(const std::string& symbol) {
  auto key = PyString_Create(symbol.c_str());
  auto value = PyDict_GetItem(g_ENV, key);
  if (value == nullptr) {
    std::cout << "[Error]: " << symbol << " is not defined." << std::endl;
    return nullptr;
  }
  return value;
}

static void
Py_ExecutePrint(const std::string& symbol) {
  auto object = Py_GetObjectBySymbol(symbol);
  if (object != nullptr)
    object->ob_type->tp_print(object);
}

static void
Py_ExecuteAdd(const std::string& target, const std::string& source) {
  auto pos = 0;
  if (_Py_IsDigit(source)) {
    auto value = PyInt_Create(std::atoi(source.c_str()));
    auto key = PyString_Create(target.c_str());
    PyDict_SetItem(g_ENV, key, value);
  }
  else if (source.find("\"") != std::string::npos) {
    auto value = PyString_Create(source.substr(1, source.size() - 2).c_str());
    auto key = PyString_Create(target.c_str());
    PyDict_SetItem(g_ENV, key, value);
  }
  else if ((pos = source.find(" + ")) != std::string::npos) {
    auto lobject = Py_GetObjectBySymbol(source.substr(0, pos));
    auto robject = Py_GetObjectBySymbol(source.substr(pos + 3));
    if (lobject != nullptr && robject != nullptr
        && lobject->ob_type == robject->ob_type) {
      auto value = lobject->ob_type->tp_add(lobject, robject);
      auto key = PyString_Create(target.c_str());
      PyDict_SetItem(g_ENV, key, value);
    }
    g_ENV->ob_type->tp_print(g_ENV);
  }
}

void Py_Execute(const std::string& command) {
  auto pos = 0;
  if ((pos = command.find("print ")) != std::string::npos) {
    Py_ExecutePrint(command.substr(6));
  }
  else if ((pos = command.find(" = ")) != std::string::npos) {
    std::string target = command.substr(0, pos);
    std::string source = command.substr(pos + 3);
    Py_ExecuteAdd(target, source);
  }
}
