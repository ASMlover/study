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
#include "nyx_container.h"

namespace nyx {

class nyx_list : public PyObject {
  using ObjectVector = std::vector<PyObject*>;
  ObjectVector* vec_{};
public:
  inline void _init(void) {
    vec_ = new ObjectVector();
  }

  inline void _dealloc(void) {
    if (vec_ != nullptr) {
      _clear();
      delete vec_;
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
};

static int _nyxlist_init(
    nyx_list* self, PyObject* /*args*/, PyObject* /*kwargs*/) {
  self->_init();
  return 0;
}

static void _nyxlist_dealloc(nyx_list* self) {
  self->_dealloc();
}

static PyObject* _nyxlist_repr(nyx_list* self) {
  return Py_BuildValue("s", self->_repr().c_str());
}

static PyObject* _nyxlist_clear(nyx_list* self) {
  self->_clear();
  Py_RETURN_NONE;
}

static PyObject* _nyxlist_size(nyx_list* self) {
  return Py_BuildValue("l", self->_size());
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

PyDoc_STRVAR(_nyxlist_doc,
"nyx_list() -> new empty nyx_list\n"
"nyx_list(iterable) -> new nyx_list initialized from iterable's items");
PyDoc_STRVAR(__clear_doc,
"L.clear() -- clear all L items");
PyDoc_STRVAR(__size_doc,
"L.size() -- return number of L items");
PyDoc_STRVAR(__append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(__insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(__getitem_doc,
"x.__getitem__(y) <==> x[y]");

static PySequenceMethods _nyxlist_as_sequence = {
  (lenfunc)_nyxlist__meth_length, // sq_length
  0, // sq_concat
  0, // sq_repeat
  (ssizeargfunc)_nyxlist__meth_item, // sq_item
  0, // sq_slice
  0, // sq_ass_item
  0, // sq_ass_slice
  (objobjproc)_nyxlist__meth_contains, // sq_contains
  0, // sq_inplace_concat
  0, // sq_inplace_repeat
};

static PyMappingMethods _nyxlist_as_mapping = {
  (lenfunc)_nyxlist__meth_length, // mp_length
  (binaryfunc)_nyxlist__meth_subscript, // mp_subscript
  0, // map_ass_subscript
};

static PyMethodDef _nyxlist_methods[] = {
  {"clear", (PyCFunction)_nyxlist_clear, METH_NOARGS, __clear_doc},
  {"size", (PyCFunction)_nyxlist_size, METH_NOARGS, __size_doc},
  {"append", (PyCFunction)_nyxlist_append, METH_O, __append_doc},
  {"insert", (PyCFunction)_nyxlist_insert, METH_VARARGS, __insert_doc},
  {"__getitem__", (PyCFunction)_nyxlist__meth_subscript, METH_O | METH_COEXIST, __getitem_doc},
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

void nyx_list_wrap(PyObject* m) {
  if (PyType_Ready(&_nyxlist_type) < 0)
    return;

  auto* _type = reinterpret_cast<PyObject*>(&_nyxlist_type);
  Py_INCREF(_type);
  PyModule_AddObject(m, "nyx_list", _type);
}

}
