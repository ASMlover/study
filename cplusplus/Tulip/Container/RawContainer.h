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

class PyTulipList : public PyObject {
  using VectorType = std::vector<PyObject*>;
  VectorType* vec_{};

  inline void _init(void) {
    vec_ = new VectorType();
  }

  inline void _dealloc(void) {
    if (vec_ != nullptr) {
      _clear();
      delete vec_;
    }
    Py_TYPE(this)->tp_free(static_cast<PyObject*>(this));
  }

  inline void _clear(void) {
    for (auto* x : *vec_)
      Py_DECREF(x);
    vec_->clear();
  }

  inline std::size_t _size(void) const {
    return vec_->size();
  }

  std::string _repr(void) const {
    std::ostringstream oss;
    oss << "[";
    auto first = vec_->begin();
    while (first != vec_->end()) {
      if (PyString_Check(*first))
        oss << "'" << PyString_AsString(*first) << "'";
      else
        oss << PyString_AsString(PyObject_Repr(*first));

      if (++first != vec_->end())
        oss << ", ";
    }
    oss << "]";

    return oss.str();
  }

  inline void _append(PyObject* x) {
    vec_->push_back(x);
  }

  inline bool _insert(Py_ssize_t i, PyObject* x) {
    try {
      i = getitem_index(i, vec_->size(), true);
    } catch (...) {
      return false;
    }

    vec_->insert(vec_->begin() + i, x);
    return true;
  }
private:
  static int _pytuliplist_init(PyTulipList* self) {
    self->_init();
    return 0;
  }

  static void _pytuliplist_dealloc(PyTulipList* self) {
    self->_dealloc();
  }

  static PyObject* _pytuliplist_clear(PyTulipList* self) {
    self->_clear();
    Py_RETURN_NONE;
  }

  static PyObject* _pytuliplist_size(PyTulipList* self) {
    return Py_BuildValue("l", self->_size());
  }

  static PyObject* _pytuliplist_repr(PyTulipList* self) {
    return Py_BuildValue("s", self->_repr().c_str());
  }

  static PyObject* _pytuliplist_append(PyTulipList* self, PyObject* args) {
    PyObject* x;
    if (!PyArg_ParseTuple(args, "O", &x))
      return nullptr;

    self->_append(x);
    Py_RETURN_NONE;
  }

  static PyObject* _pytuliplist_insert(PyTulipList* self, PyObject* args) {
    Py_ssize_t i;
    PyObject* x;
    if (!PyArg_ParseTuple(args, "lO", &i, &x))
      return nullptr;

    if (!self->_insert(i, x)) {
      PyErr_SetString(PyExc_IndexError, "insert(...): index out of range.");
      return nullptr;
    }
    Py_RETURN_NONE;
  }
public:
  static void wrap(PyObject* m) {
    static PyMethodDef _pytuliplist_methods[] = {
      {"clear", (PyCFunction)_pytuliplist_clear, METH_NOARGS, "clear()"},
      {"size", (PyCFunction)_pytuliplist_size, METH_NOARGS, "size()"},
      {"append", (PyCFunction)_pytuliplist_append, METH_VARARGS, "append(...)"},
      {"insert", (PyCFunction)_pytuliplist_insert, METH_VARARGS, "insert(...)"},
      {nullptr}
    };

    static PyTypeObject _PyTulipList_Type = {
      PyObject_HEAD_INIT(nullptr)
      0, // ob_size
      "PyTulipList", // tp_name
      sizeof(PyTulipList), // tp_basicsize
      0, // tp_itemsize
      (destructor)_pytuliplist_dealloc, // tp_dealloc
      0, // tp_print
      0, // tp_getattr
      0, // tp_setattr
      0, // tp_compare
      (reprfunc)_pytuliplist_repr, // tp_repr
      0, // tp_as_number
      0, // tp_as_sequence
      0, // tp_as_mapping
      0, // tp_hash
      0, // tp_call
      (reprfunc)_pytuliplist_repr, // tp_str
      0, // tp_getattro
      0, // tp_setattro
      0, // tp_as_buffer
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
      "class PyTulipList", // tp_doc
      0, // tp_traverse
      0, // tp_clear
      0, // tp_richcompare
      0, // tp_weaklistoffset
      0, // tp_iter
      0, // tp_iternext
      _pytuliplist_methods, // tp_methods
      0, // tp_members
      0, // tp_getset
      0, // tp_base
      0, // tp_dict
      0, // tp_descr_get
      0, // tp_descr_set
      0, // tp_dictoffset
      (initproc)_pytuliplist_init, // tp_init
      0, // tp_alloc
      PyType_GenericNew, // tp_new
    };

    if (PyType_Ready(&_PyTulipList_Type) < 0)
      return;

    auto* _type = reinterpret_cast<PyObject*>(&_PyTulipList_Type);
    Py_INCREF(_type);
    PyModule_AddObject(m, "PyTulipList", _type);
  }
};

}
