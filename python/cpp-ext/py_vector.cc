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
#include <sstream>
#include <vector>
#include "py_vector.h"

class CppVector {
  std::vector<PyObject*> vec_;
public:
  std::string as_string(void) const {
    std::ostringstream oss;
    oss << "[";
    auto first = vec_.begin();
    while (first != vec_.end()) {
      if (PyString_Check(*first))
        oss << "'" << PyString_AsString(*first) << "'";
      else
        oss << PyString_AsString(PyObject_Repr(*first));

      if (++first != vec_.end())
        oss << ", ";
    }
    oss << "]";

    return oss.str();
  }

  inline void clear(void) {
    for (auto* o : vec_)
      Py_DECREF(o);
    vec_.clear();
  }

  inline std::size_t size(void) const {
    return vec_.size();
  }

  inline void append(PyObject* x) {
    Py_INCREF(x);
    vec_.push_back(x);
  }
};

static int pyvector_init(PyVector* self) {
  self->c_obj = new CppVector();
  return 0;
}

static void pyvector_dealloc(PyVector* self) {
  if (self->c_obj != nullptr) {
    self->c_obj->clear();
    delete self->c_obj;
  }
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* pyvector_repr(PyVector* self) {
  return Py_BuildValue("s", self->c_obj->as_string().c_str());
}

static PyObject* pyvector_clear(PyVector* self) {
  self->c_obj->clear();
  Py_RETURN_NONE;
}

static PyObject* pyvector_size(PyVector* self) {
  auto n = self->c_obj->size();
  return Py_BuildValue("l", n);
}

static PyObject* pyvector_append(PyVector* self, PyObject* args) {
  PyObject* x;
  if (!PyArg_ParseTuple(args, "O", &x))
    return NULL;

  self->c_obj->append(x);
  Py_RETURN_NONE;
}

static PyMethodDef _pyvector_methods[] = {
  {"clear", (PyCFunction)pyvector_clear, METH_NOARGS, "clear"},
  {"size", (PyCFunction)pyvector_size, METH_NOARGS, "size"},
  {"append", (PyCFunction)pyvector_append, METH_VARARGS, "append"},
  {NULL, NULL, NULL, NULL}
};

static PyTypeObject _PyVector_Type = {
  PyObject_HEAD_INIT(NULL)
  0, // ob_size
  "PyVector", // tp_name
  sizeof(PyVector), // tp_basicsize
  0, // tp_itemsize
  (destructor)pyvector_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  (reprfunc)pyvector_repr, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  (reprfunc)pyvector_repr, // tp_str
  0, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  "", // tp_doc
  0, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  _pyvector_methods, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // &PyType_Type, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  (initproc)pyvector_init, // tp_init
  0, // tp_alloc
  PyType_GenericNew, // tp_new
};

void init_pyvector(void) {
  if (PyType_Ready(&_PyVector_Type) < 0)
    return;

  static PyMethodDef _cppext_methods[] = {
    {NULL}
  };
  auto* m = Py_InitModule3("_cppext", _cppext_methods, "cpp ext-library.");

  Py_INCREF(&_PyVector_Type);
  PyModule_AddObject(m, "PyVector", (PyObject*)&_PyVector_Type);
}
