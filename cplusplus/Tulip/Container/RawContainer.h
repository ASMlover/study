// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <vector>
#include "../Utility.h"

namespace tulip {

struct PyTulipList : public PyObject {
  std::vector<PyObject*>* elements_{};

  void py_init(void) {
    elements_ = new std::vector<PyObject*>();
  }

  void py_dealloc(void) {
    if (elements_ != nullptr) {
      for (auto* x : *elements_)
        Py_DECREF(x);
      elements_->clear();

      delete elements_;
      elements_ = nullptr;
    }
  }

  std::string py_repr(void) const {
    std::ostringstream oss;
    oss << "[";
    bool is_first{true};
    for (auto* x : *elements_) {
      if (is_first)
        is_first = false;
      else
        oss << ", ";

      if (PyString_Check(x))
        oss << "'" << PyString_AsString(x) << "'";
      else
        oss << PyString_AsString(PyObject_Str(x));
    }
    oss << "]";

    return oss.str();
  }

  static void pytuliplist_init(PyTulipList* self, PyObject* args) {
    self->py_init();
  }

  static void pytuliplist_dealloc(PyTulipList* self) {
    self->py_dealloc();
  }

  static PyObject* pytuliplist_repr(PyTulipList* self) {
    return Py_BuildValue("s", self->py_repr().c_str());
  }

  static PyObject* pytuliplist_str(PyTulipList* self) {
    return pytuliplist_repr(self);
  }

  static void wrap(PyObject* module) {
    static PyMethodDef pytuliplist_methods[] = {
      {NULL, NULL},
    };

    static PyTypeObject PyTulipList_Type = {
      PyObject_HEAD_INIT(NULL)
      0, // ob_size
      "PyTulipList", // tp_name
      sizeof(PyTulipList), // tp_basicsize
      0, // tp_itemsize
      (destructor)pytuliplist_dealloc, // tp_dealloc
      0, // tp_print
      0, // tp_getattr
      0, // tp_setattr
      0, // tp_compare
      (reprfunc)pytuliplist_repr, // tp_repr
      0, // tp_as_number
      0, // tp_as_sequence
      0, // tp_as_mapping
      0, // tp_hash
      0, // tp_call
      (reprfunc)pytuliplist_str, // tp_str
      0, // tp_getattro
      0, // tp_setattro
      0, // tp_as_buffer
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
      "no doc", // tp_doc
      0, // tp_traverse
      0, // tp_clear
      0, // tp_richcompare
      0, // tp_weaklistoffset
      0, // tp_iter
      0, // tp_iternext
      pytuliplist_methods, // tp_methods
      0, // tp_members
      0, // tp_getset
      &PyType_Type, // tp_base
      0, // tp_dict
      0, // tp_descr_get
      0, // tp_descr_set
      0, // tp_dictoffset
      (initproc)pytuliplist_init, // tp_init
      0, // tp_alloc
      PyType_GenericNew, // tp_new
    };

    if (PyType_Ready(&PyTulipList_Type) < 0)
      return;

    Py_INCREF(&PyTulipList_Type);
    PyModule_AddObject(module, "PyTulipList", (PyObject*)&PyTulipList_Type);
  }
};

}
