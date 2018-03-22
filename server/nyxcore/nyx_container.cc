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
#include <algorithm>
#include <sstream>
#include <vector>
#include "nyx_container.h"

namespace nyx {

class nyx_list : public PyObject {
  using ObjectVector = std::vector<PyObject*>;
  ObjectVector* vec_{};
public:
  inline void _init(void) {
    if (vec_ != nullptr)
      _clear();
    else
      vec_ = new ObjectVector();
  }

  inline void _dealloc(void) {
    if (vec_ != nullptr) {
      _clear();
      delete vec_;
    }
  }

  inline void _copy(nyx_list* other) {
    other->_clear();
    for (auto* x : *vec_) {
      Py_INCREF(x);
      other->_append(x);
    }
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

  inline void _clear(void) {
    for (auto* x : *vec_)
      Py_DECREF(x);
    vec_->clear();
  }

  inline Py_ssize_t _size(void) const {
    return vec_->size();
  }

  inline Py_ssize_t _capacity_bytes(void) const {
    return vec_->capacity() * sizeof(PyObject*);
  }

  std::string _repr(void) const {
    std::ostringstream oss;
    oss << "[";
    auto begin = vec_->begin();
    while (begin != vec_->end()) {
      if (PyString_Check(*begin))
        oss << "'" << PyString_AsString(*begin) << "'";
      else
        oss << PyString_AsString(PyObject_Repr(*begin));

      if (++begin != vec_->end())
        oss << ", ";
    }
    oss << "]";

    return oss.str();
  }

  inline void _append(PyObject* x) {
    vec_->push_back(x);
  }

  inline void _insert(Py_ssize_t i, PyObject* x) {
    vec_->insert(vec_->begin() + i, x);
  }

  inline PyObject* _at(Py_ssize_t i) const {
    return vec_->at(i);
  }

  inline PyObject* _pop(Py_ssize_t i) {
    auto* v = (*vec_)[i];
    vec_->erase(vec_->begin() + i);
    return v;
  }

  inline bool _remove(PyObject* v) {
    for (auto it = vec_->begin(); it != vec_->end(); ++it) {
      if (PyObject_RichCompareBool(v, *it, Py_EQ) > 0) {
        Py_DECREF(*it);
        vec_->erase(it);
        return true;
      }
    }
    return false;
  }

  inline void _setitem(Py_ssize_t i, PyObject* v) {
    (*vec_)[i] = v;
  }

  inline Py_ssize_t _index(PyObject* v, Py_ssize_t start, Py_ssize_t stop) {
    for (auto i = start; i < stop && i < _size(); ++i) {
      auto cmp = PyObject_RichCompareBool((*vec_)[i], v, Py_EQ);
      if (cmp > 0)
        return i;
      else if (cmp < 0)
        return -1;
    }
    return 0;
  }

  inline Py_ssize_t _count(PyObject* v) const {
    Py_ssize_t count = 0;
    for (auto* x : *vec_) {
      auto cmp = PyObject_RichCompareBool(x, v, Py_EQ);
      if (cmp > 0)
        ++count;
      else if (cmp < 0)
        return -1;
    }
    return count;
  }

  inline void _reverse(void) {
    std::reverse(std::begin(*vec_), std::end(*vec_));
  }
};

static bool __is_nyxlist(PyObject* o);
static PyObject* __nyxlist_new(void);
static PyObject* _nyxlist_extend(nyx_list* self, PyObject* other);

static int _nyxlist_init(
    nyx_list* self, PyObject* args, PyObject* kwargs) {
  PyObject* arg{};
  static char* kwlist[] = {"sequence", 0};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:nyx_list", kwlist, &arg))
    return -1;

  self->_init();
  if (arg != nullptr) {
    auto* r = _nyxlist_extend(self, arg);
    if (r == nullptr)
      return -1;
    Py_DECREF(r);
  }
  return 0;
}

static void _nyxlist_dealloc(nyx_list* self) {
  self->_dealloc();
}

static PyObject* _nyxlist_repr(nyx_list* self) {
  return Py_BuildValue("s", self->_repr().c_str());
}

static PyObject* _nyxlist_copy(nyx_list* self) {
  nyx_list* copy_self = (nyx_list*)__nyxlist_new();
  self->_copy(copy_self);
  return copy_self;
}

static PyObject* _nyxlist_clear(nyx_list* self) {
  self->_clear();
  Py_RETURN_NONE;
}

static PyObject* _nyxlist_size(nyx_list* self) {
  return Py_BuildValue("l", self->_size());
}

static PyObject* _nyxlist_sizeof(nyx_list* self) {
  auto s = _PyObject_SIZE(Py_TYPE(self)) + self->_capacity_bytes();
  return PyInt_FromSsize_t(s);
}

static PyObject* _nyxlist_append(nyx_list* self, PyObject* v) {
  if (v == nullptr)
    return nullptr;
  if (self == v) {
    PyErr_SetString(PyExc_RuntimeError, "append(object): can not append self");
    return nullptr;
  }

  Py_INCREF(v);
  self->_append(v);
  Py_RETURN_NONE;
}

static PyObject* _nyxlist_insert(nyx_list* self, PyObject* args) {
  Py_ssize_t i;
  PyObject* v;
  if (!PyArg_ParseTuple(args, "nO:insert", &i, &v))
    return nullptr;
  if (v == nullptr) {
    PyErr_BadInternalCall();
    return nullptr;
  }
  if (self == v) {
    PyErr_SetString(PyExc_RuntimeError,
        "insert(index, object): cannot insert self");
    return nullptr;
  }

  auto n = self->_size();
  if (n == PY_SSIZE_T_MAX) {
    PyErr_SetString(PyExc_OverflowError,
        "insert(index, object): cannot add more objects to list");
    return nullptr;
  }
  if (i < 0)
    i += n;
  if (i < 0 || i > n) {
    PyErr_SetString(PyExc_IndexError,
        "insert(index, object): index out of range");
    return nullptr;
  }

  Py_INCREF(v);
  self->_insert(i, v);
  Py_RETURN_NONE;
}

static PyObject* _nyxlist_extend(nyx_list* self, PyObject* other) {
  if (self == other) {
    PyErr_SetString(PyExc_RuntimeError, "extend(iterable): cannot extend self");
    return nullptr;
  }

  if (PyTuple_CheckExact(other) || PyList_CheckExact(other)) {
    other = PySequence_Fast(other, "argument must be iterable");
    if (other == nullptr)
      return nullptr;
    auto n = PySequence_Fast_GET_SIZE(other);
    if (n == 0) {
      Py_DECREF(other);
      Py_RETURN_NONE;
    }

    auto* src = PySequence_Fast_ITEMS(other);
    for (auto i = 0; i < n; ++i) {
      auto* x = src[i];
      Py_INCREF(x);
      self->_append(x);
    }
    Py_DECREF(other);
  }
  else if (__is_nyxlist(other)) {
    auto* other_self = static_cast<nyx_list*>(other);
    auto n = other_self->_size();
    if (n > 0) {
      for (auto i = 0; i < n; ++i) {
        auto* x = other_self->_at(i);
        Py_INCREF(x);
        self->_append(x);
      }
    }
  }
  else {
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject* _nyxlist_pop(nyx_list* self, PyObject* args) {
  Py_ssize_t i{-1};

  if (!PyArg_ParseTuple(args, "|n:pop", &i))
    return nullptr;

  auto n = self->_size();
  if (n == 0) {
    PyErr_SetString(PyExc_IndexError, "pop from empty list");
    return nullptr;
  }
  if (i < 0)
    i += n;
  if (i < 0 || i >= n) {
    PyErr_SetString(PyExc_IndexError, "pop index out of range");
    return nullptr;
  }

  return self->_pop(i);
}

static PyObject* _nyxlist_remove(nyx_list* self, PyObject* v) {
  if (self->_remove(v))
    Py_RETURN_NONE;

  PyErr_SetString(PyExc_ValueError, "L.remove(value): value is not in list");
  return nullptr;
}

static PyObject* _nyxlist_index(nyx_list* self, PyObject* args) {
  PyObject* v;
  Py_ssize_t start = 0;
  Py_ssize_t n = self->_size();
  Py_ssize_t stop = n;

  if (!PyArg_ParseTuple(args, "O|O&O:index", &v,
        _PyEval_SliceIndex, &start, _PyEval_SliceIndex, &stop))
    return nullptr;

  if (start < 0) {
    start += n;
    if (start < 0)
      start = 0;
  }
  if (stop < 0) {
    stop += n;
    if (stop < 0)
      stop = 0;
  }
  auto i = self->_index(v, start, stop);
  if (i > 0)
    return PyInt_FromSsize_t(i);
  else if (i < 0)
    return nullptr;

  PyErr_SetString(PyExc_ValueError, "index(value): value is not in list");
  return nullptr;
}

static PyObject* _nyxlist_count(nyx_list* self, PyObject* v) {
  auto count = self->_count(v);
  if (count < 0)
    return nullptr;
  return PyInt_FromSsize_t(count);
}

static PyObject* _nyxlist_reverse(nyx_list* self) {
  if (self->_size() > 0)
    self->_reverse();
  Py_RETURN_NONE;
}

static Py_ssize_t _nyxlist__meth_length(nyx_list* self) {
  return self->_size();
}

static PyObject* _nyxlist__meth_item(nyx_list* self, Py_ssize_t i) {
  if (i < 0 || i >= self->_size()) {
    PyErr_SetString(PyExc_IndexError, "list index out of range");
    return nullptr;
  }

  auto* v = self->_at(i);
  Py_INCREF(v);
  return v;
}

static int _nyxlist__meth_ass_item(nyx_list* self, Py_ssize_t i, PyObject* v) {
  if (i < 0 || i >= self->_size()) {
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

static int _nyxlist__meth_contains(nyx_list* self, PyObject* o) {
  return self->_contains(o);
}

static PyObject* _nyxlist__meth_subscript(nyx_list* self, PyObject* index) {
  if (PyIndex_Check(index)) {
    auto i = PyNumber_AsSsize_t(index, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred())
      return nullptr;
    if (i < 0)
      i += self->_size();
    return _nyxlist__meth_item(self, i);
  }
  else {
    PyErr_Format(PyExc_TypeError,
        "list indices must be integers, not %.200s", index->ob_type->tp_name);
    return nullptr;
  }
}

static int _nyxlist__meth_ass_subscript(
    nyx_list* self, PyObject* index, PyObject* v) {
  if (PyIndex_Check(index)) {
    auto i = PyNumber_AsSsize_t(index, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred())
      return -1;
    if (i < 0)
      i += self->_size();
    return _nyxlist__meth_ass_item(self, i, v);
  }
  else {
    PyErr_Format(PyExc_TypeError,
        "list indices must be integers, not %.200s", index->ob_type->tp_name);
    return -1;
  }
}

PyDoc_STRVAR(_nyxlist_doc,
"nyx_list() -> new empty nyx_list\n"
"nyx_list(iterable) -> new nyx_list initialized from iterable's items");
PyDoc_STRVAR(__copy_doc,
"L.copy() -> a shallow copy of L");
PyDoc_STRVAR(__clear_doc,
"L.clear() -- clear all elements of L");
PyDoc_STRVAR(__size_doc,
"L.size() -- return number of elements in L");
PyDoc_STRVAR(__append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(__insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(__extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable");
PyDoc_STRVAR(__pop_doc,
"L.pop([index]) -> item -- remove and return item at index (default last).\n"
"Raises IndexError if last is empty or index is out of range.");
PyDoc_STRVAR(__remove_doc,
"L.remove(value) -- remove first occurrence of value.\n"
"Raises ValueError if the value is not present.");
PyDoc_STRVAR(__index_doc,
"L.index(value, [start, [stop]]) -> integer -- return first index of value.\n"
"Raises ValueError if the value is not present.");
PyDoc_STRVAR(__count_doc,
"L.count(value) -> integer -- return number of occurrences of value");
PyDoc_STRVAR(__reverse_doc,
"L.reverse() -- reverse *IN PLACE*");
PyDoc_STRVAR(__getitem_doc,
"x.__getitem__(y) <==> x[y]");
PyDoc_STRVAR(__sizeof_doc,
"L.__sizeof__() -- size of L in memory, in bytes");

static PySequenceMethods _nyxlist_as_sequence = {
  (lenfunc)_nyxlist__meth_length, // sq_length
  0, // sq_concat
  0, // sq_repeat
  (ssizeargfunc)_nyxlist__meth_item, // sq_item
  0, // sq_slice
  (ssizeobjargproc)_nyxlist__meth_ass_item, // sq_ass_item
  0, // sq_ass_slice
  (objobjproc)_nyxlist__meth_contains, // sq_contains
  0, // sq_inplace_concat
  0, // sq_inplace_repeat
};

static PyMappingMethods _nyxlist_as_mapping = {
  (lenfunc)_nyxlist__meth_length, // mp_length
  (binaryfunc)_nyxlist__meth_subscript, // mp_subscript
  (objobjargproc)_nyxlist__meth_ass_subscript, // map_ass_subscript
};

static PyMethodDef _nyxlist_methods[] = {
  {"copy", (PyCFunction)_nyxlist_copy, METH_NOARGS, __copy_doc},
  {"clear", (PyCFunction)_nyxlist_clear, METH_NOARGS, __clear_doc},
  {"size", (PyCFunction)_nyxlist_size, METH_NOARGS, __size_doc},
  {"append", (PyCFunction)_nyxlist_append, METH_O, __append_doc},
  {"insert", (PyCFunction)_nyxlist_insert, METH_VARARGS, __insert_doc},
  {"extend", (PyCFunction)_nyxlist_extend, METH_O, __extend_doc},
  {"pop", (PyCFunction)_nyxlist_pop, METH_VARARGS, __pop_doc},
  {"remove", (PyCFunction)_nyxlist_remove, METH_O, __remove_doc},
  {"index", (PyCFunction)_nyxlist_index, METH_VARARGS, __index_doc},
  {"count", (PyCFunction)_nyxlist_count, METH_O, __count_doc},
  {"reverse", (PyCFunction)_nyxlist_reverse, METH_NOARGS, __reverse_doc},
  {"__getitem__", (PyCFunction)_nyxlist__meth_subscript, METH_O | METH_COEXIST, __getitem_doc},
  {"__sizeof__", (PyCFunction)_nyxlist_sizeof, METH_NOARGS, __sizeof_doc},
  {nullptr}
};

static PyTypeObject _nyxlist_type = {
  PyObject_HEAD_INIT(nullptr)
  0, // ob_size
  "_nyxcore.nyx_list", // tp_name
  sizeof(nyx_list), // tp_basicsize
  0, // tp_itemsize
  (destructor)_nyxlist_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  (reprfunc)_nyxlist_repr, // tp_repr
  0, // tp_as_number
  &_nyxlist_as_sequence, // tp_as_sequence
  &_nyxlist_as_mapping, // tp_as_mapping
  (hashfunc)PyObject_HashNotImplemented, // tp_hash
  0, // tp_call
  (reprfunc)_nyxlist_repr, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  _nyxlist_doc, // tp_doc
  0, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  _nyxlist_methods, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  (initproc)_nyxlist_init, // tp_init
  0, // tp_alloc
  PyType_GenericNew, // tp_new
};

static bool __is_nyxlist(PyObject* o) {
  return Py_TYPE(o) == &_nyxlist_type;
}

static PyObject* __nyxlist_new(void) {
  PyTypeObject* _type = &_nyxlist_type;
  auto* nl = (nyx_list*)_type->tp_alloc(_type, 0);
  if (nl != nullptr)
    nl->_init();
  return nl;
}

void nyx_list_wrap(PyObject* m) {
  if (PyType_Ready(&_nyxlist_type) < 0)
    return;

  auto* _type = reinterpret_cast<PyObject*>(&_nyxlist_type);
  Py_INCREF(_type);
  PyModule_AddObject(m, "nyx_list", _type);
}

}
