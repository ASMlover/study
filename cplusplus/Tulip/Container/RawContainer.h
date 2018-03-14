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

  inline PyObject* _pop(std::size_t i) {
    auto* v = (*vec_)[i];
    vec_->erase(vec_->begin() + i);
    return v;
  }

  inline void _foreach(PyObject* callable) {
    for (auto* x : *vec_) {
      auto* args = Py_BuildValue("(O)", x);
      auto* r = PyObject_CallObject(callable, args);
      Py_DECREF(args);
      Py_DECREF(r);
    }
  }

  inline PyObject* _at(Py_ssize_t i) const {
    return vec_->at(i);
  }

  inline void _setitem(Py_ssize_t i, PyObject* v) {
    vec_->at(i) = v;
  }

  inline int _contains(PyObject* o) const {
    int cmp{};
    for (auto* x : *vec_) {
      cmp = PyObject_RichCompareBool(o, x, Py_EQ);
      if (cmp != 0)
        break;
    }
    return cmp;
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

  static PyObject* _pytuliplist_contains(PyTulipList* self, PyObject* x) {
    auto cmp = self->_contains(x);
    if (cmp == -1)
      return nullptr;
    else if (cmp == 0)
      Py_RETURN_FALSE;
    else
      Py_RETURN_TRUE;
  }

  static PyObject* _pytuliplist_repr(PyTulipList* self) {
    return Py_BuildValue("s", self->_repr().c_str());
  }

  static PyObject* _pytuliplist_append(PyTulipList* self, PyObject* x) {
    if (self == x) {
      PyErr_SetString(PyExc_RuntimeError, "append(x): can not append self");
      return nullptr;
    }
    self->_append(x);
    Py_RETURN_NONE;
  }

  static PyObject* _pytuliplist_insert(PyTulipList* self, PyObject* args) {
    Py_ssize_t i;
    PyObject* x;
    if (!PyArg_ParseTuple(args, "nO:insert", &i, &x))
      return nullptr;

    if (self == x) {
      PyErr_SetString(PyExc_RuntimeError, "insert(i, x): can not insert self");
      return nullptr;
    }
    if (!self->_insert(i, x)) {
      PyErr_SetString(PyExc_IndexError, "insert(...): index out of range");
      return nullptr;
    }
    Py_RETURN_NONE;
  }

  static PyObject* _pytuliplist_pop(PyTulipList* self, PyObject* args) {
    Py_ssize_t i = -1;

    if (!PyArg_ParseTuple(args, "|n:pop", &i))
      return nullptr;

    auto n = self->_size();
    if (n == 0) {
      PyErr_SetString(PyExc_IndexError, "pop(...): pop from empty list");
      return nullptr;
    }
    if (i < 0)
      i += n;
    if (i < 0 || i >= (Py_ssize_t)n) {
      PyErr_SetString(PyExc_IndexError, "pop(...): pop index out of range");
      return nullptr;
    }

    return self->_pop(i);
  }

  static PyObject* _pytuliplist_foreach(PyTulipList* self, PyObject* callable) {
    if (callable != nullptr && callable != Py_None)
      self->_foreach(callable);
    Py_RETURN_NONE;
  }

  static Py_ssize_t _pytuliplist__fun_length(PyTulipList* self) {
    return self->_size();
  }

  static PyObject* _pytuliplist__fun_item(PyTulipList* self, Py_ssize_t i) {
    if (i < 0 || i >= (Py_ssize_t)self->_size()) {
      PyErr_SetString(PyExc_IndexError, "list index out of range");
      return nullptr;
    }

    auto* x = self->_at(i);
    Py_INCREF(x);
    return x;
  }

  static int _pytuliplist__fun_ass_item(
      PyTulipList* self, Py_ssize_t i, PyObject* v) {
    if (i < 0 || i >= (Py_ssize_t)self->_size()) {
      PyErr_SetString(PyExc_IndexError, "list assignment index out of range");
      return -1;
    }
    if (v == nullptr || self == v) {
      PyErr_SetString(PyExc_RuntimeError, "list assignment invalid value");
      return -1;
    }

    Py_INCREF(v);
    auto* old = self->_at(i);
    self->_setitem(i, v);
    Py_DECREF(old);
    return 0;
  }

  static int _pytuliplist__fun_contains(PyTulipList* self, PyObject* o) {
    return self->_contains(o);
  }

  static PyObject* _pytuliplist__fun_subscript(PyTulipList* self, PyObject* index) {
    if (PyIndex_Check(index)) {
      auto i = PyNumber_AsSsize_t(index, PyExc_IndexError);
      if (i == -1 && PyErr_Occurred())
        return nullptr;
      if (i < 0)
        i += self->_size();
      return _pytuliplist__fun_item(self, i);
    }
    else {
      PyErr_Format(PyExc_TypeError,
          "subscript(...): list indices must be integers, not %.200s",
          index->ob_type->tp_name);
      return nullptr;
    }
  }

  static int _pytuliplist__fun_ass_subscript(
      PyTulipList* self, PyObject* index, PyObject* v) {
    if (PyIndex_Check(index)) {
      Py_ssize_t i = PyNumber_AsSsize_t(index, PyExc_IndexError);
      if (i == -1 && PyErr_Occurred())
        return -1;
      if (i < 0)
        i += self->_size();
      return _pytuliplist__fun_ass_item(self, i, v);
    }
    else {
      PyErr_Format(PyExc_TypeError,
          "ass_subscript(...): list indices must be integers, not %.200s",
          index->ob_type->tp_name);
      return -1;
    }
  }
public:
  static void wrap(PyObject* m) {
    static PySequenceMethods _pytuliplist_as_sequence = {
      (lenfunc)_pytuliplist__fun_length, // sq_length
      0, // sq_concat
      0, // sq_repeat
      (ssizeargfunc)_pytuliplist__fun_item, // sq_item
      0, // sq_slice
      (ssizeobjargproc)_pytuliplist__fun_ass_item, // sq_ass_item
      0, // sq_ass_slice
      (objobjproc)_pytuliplist__fun_contains, // sq_contains
      0, // sq_inplace_concat
      0, // sq_inplace_repeat
    };

    static PyMappingMethods _pytuliplist_as_mapping = {
      (lenfunc)_pytuliplist__fun_length, // mp_length
      (binaryfunc)_pytuliplist__fun_subscript, // mp_subscript
      (objobjargproc)_pytuliplist__fun_ass_subscript, // mp_ass_subscript
    };

    static PyMethodDef _pytuliplist_methods[] = {
      {"clear", (PyCFunction)_pytuliplist_clear, METH_NOARGS, "clear()"},
      {"size", (PyCFunction)_pytuliplist_size, METH_NOARGS, "size()"},
      {"contains", (PyCFunction)_pytuliplist_contains, METH_O, "contains(...)"},
      {"append", (PyCFunction)_pytuliplist_append, METH_O, "append(...)"},
      {"insert", (PyCFunction)_pytuliplist_insert, METH_VARARGS, "insert(...)"},
      {"pop", (PyCFunction)_pytuliplist_pop, METH_VARARGS, "pop(...)"},
      {"foreach", (PyCFunction)_pytuliplist_foreach, METH_O, "foreach(...)"},
      {"__getitem__", (PyCFunction)_pytuliplist__fun_subscript,
        METH_O | METH_COEXIST, "__getitem__(...)"},
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
      &_pytuliplist_as_sequence, // tp_as_sequence
      &_pytuliplist_as_mapping, // tp_as_mapping
      (hashfunc)PyObject_HashNotImplemented, // tp_hash
      0, // tp_call
      (reprfunc)_pytuliplist_repr, // tp_str
      PyObject_GenericGetAttr, // tp_getattro
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
