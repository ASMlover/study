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
#include <unordered_map>
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

  int _traverse(visitproc visit, void* arg) {
    for (auto* x : *vec_)
      Py_VISIT(x);
    return 0;
  }
};

static bool __is_nyxlist(PyObject* o);
static PyObject* __nyxlist_new(void);
static PyObject* _nyxlist_extend(nyx_list* self, PyObject* other);

static int _nyxlist_tp_init(nyx_list* self, PyObject* args, PyObject* kwargs) {
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

static void _nyxlist_tp_dealloc(nyx_list* self) {
  self->_dealloc();
  self->ob_type->tp_free(self);
}

static PyObject* _nyxlist_tp_repr(nyx_list* self) {
  return Py_BuildValue("s", self->_repr().c_str());
}

static int _nyxlist_tp_traverse(nyx_list* self, visitproc visit, void* arg) {
  return self->_traverse(visit, arg);
}

static int _nyxlist_tp_clear(nyx_list* self) {
  self->_clear();
  return 0;
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
PyDoc_STRVAR(__nyxlist_copy_doc,
"L.copy() -> a shallow copy of L");
PyDoc_STRVAR(__nyxlist_clear_doc,
"L.clear() -- remove all item of L");
PyDoc_STRVAR(__nyxlist_size_doc,
"L.size() -> integer -- return number of items in L");
PyDoc_STRVAR(__nyxlist_append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(__nyxlist_insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(__nyxlist_extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable");
PyDoc_STRVAR(__nyxlist_pop_doc,
"L.pop([index]) -> item -- remove and return item at index (default last).\n"
"Raises IndexError if last is empty or index is out of range.");
PyDoc_STRVAR(__nyxlist_remove_doc,
"L.remove(value) -- remove first occurrence of value.\n"
"Raises ValueError if the value is not present.");
PyDoc_STRVAR(__nyxlist_index_doc,
"L.index(value, [start, [stop]]) -> integer -- return first index of value.\n"
"Raises ValueError if the value is not present.");
PyDoc_STRVAR(__nyxlist_count_doc,
"L.count(value) -> integer -- return number of occurrences of value");
PyDoc_STRVAR(__nyxlist_reverse_doc,
"L.reverse() -- reverse *IN PLACE*");
PyDoc_STRVAR(__nyxlist_getitem_doc,
"x.__getitem__(y) <==> x[y]");
PyDoc_STRVAR(__nyxlist_sizeof_doc,
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
  {"copy", (PyCFunction)_nyxlist_copy, METH_NOARGS, __nyxlist_copy_doc},
  {"clear", (PyCFunction)_nyxlist_clear, METH_NOARGS, __nyxlist_clear_doc},
  {"size", (PyCFunction)_nyxlist_size, METH_NOARGS, __nyxlist_size_doc},
  {"append", (PyCFunction)_nyxlist_append, METH_O, __nyxlist_append_doc},
  {"insert", (PyCFunction)_nyxlist_insert, METH_VARARGS, __nyxlist_insert_doc},
  {"extend", (PyCFunction)_nyxlist_extend, METH_O, __nyxlist_extend_doc},
  {"pop", (PyCFunction)_nyxlist_pop, METH_VARARGS, __nyxlist_pop_doc},
  {"remove", (PyCFunction)_nyxlist_remove, METH_O, __nyxlist_remove_doc},
  {"index", (PyCFunction)_nyxlist_index, METH_VARARGS, __nyxlist_index_doc},
  {"count", (PyCFunction)_nyxlist_count, METH_O, __nyxlist_count_doc},
  {"reverse", (PyCFunction)_nyxlist_reverse, METH_NOARGS, __nyxlist_reverse_doc},
  {"__getitem__", (PyCFunction)_nyxlist__meth_subscript, METH_O | METH_COEXIST, __nyxlist_getitem_doc},
  {"__sizeof__", (PyCFunction)_nyxlist_sizeof, METH_NOARGS, __nyxlist_sizeof_doc},
  {nullptr}
};

static PyTypeObject _nyxlist_type = {
  PyObject_HEAD_INIT(nullptr)
  0, // ob_size
  "_nyxcore.nyx_list", // tp_name
  sizeof(nyx_list), // tp_basicsize
  0, // tp_itemsize
  (destructor)_nyxlist_tp_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  (reprfunc)_nyxlist_tp_repr, // tp_repr
  0, // tp_as_number
  &_nyxlist_as_sequence, // tp_as_sequence
  &_nyxlist_as_mapping, // tp_as_mapping
  (hashfunc)PyObject_HashNotImplemented, // tp_hash
  0, // tp_call
  (reprfunc)_nyxlist_tp_repr, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE, // tp_flags
  _nyxlist_doc, // tp_doc
  (traverseproc)_nyxlist_tp_traverse, // tp_traverse
  (inquiry)_nyxlist_tp_clear, // tp_clear
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
  (initproc)_nyxlist_tp_init, // tp_init
  PyType_GenericAlloc, // tp_alloc
  PyType_GenericNew, // tp_new
  PyObject_GC_Del, // tp_free
};

bool __is_nyxlist(PyObject* o) {
  return Py_TYPE(o) == &_nyxlist_type;
}

PyObject* __nyxlist_new(void) {
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

class nyx_dict : public PyObject {
  using ObjectMap = std::unordered_map<long, std::pair<PyObject*, PyObject*>>;
  using IterType = ObjectMap::const_iterator;
  ObjectMap* map_{};
public:
  inline void _init(void) {
    if (map_ != nullptr)
      _clear();
    else
      map_ = new ObjectMap();
  }

  inline void _dealloc(void) {
    if (map_ != nullptr) {
      _clear();
      delete map_;
    }
  }

  inline std::string __as_string(PyObject* x) const {
    if (PyString_Check(x))
      return std::string("'") + PyString_AsString(x) + "'";
    else
      return PyString_AsString(PyObject_Repr(x));
  }

  std::string _repr(void) const {
    std::ostringstream oss;
    oss << "{";
    auto begin = map_->begin();
    while (begin != map_->end()) {
      oss << __as_string(begin->second.first) << ": "
          << __as_string(begin->second.second);

      if (++begin != map_->end())
        oss << ", ";
    }
    oss << "}";

    return oss.str();
  }

  inline void _clear(void) {
    for (auto& x : *map_) {
      Py_DECREF(x.second.first);
      Py_DECREF(x.second.second);
    };
    map_->clear();
  }

  inline Py_ssize_t _size(void) const {
    return map_->size();
  }

  inline bool _contains(long k) const {
    return map_->find(k) != map_->end();
  }

  inline PyObject* _get(long k) const {
    PyObject* v{};
    auto pos = map_->find(k);
    if (pos != map_->end())
      v = pos->second.second;
    return v;
  }

  inline PyObject* _setdefault(long hash_code, PyObject* k, PyObject* v) {
    PyObject* r = v;

    Py_INCREF(k);
    Py_INCREF(v);
    auto pos = map_->find(hash_code);
    if (pos != map_->end()) {
      r = pos->second.second;
      Py_DECREF(pos->second.first);
      Py_DECREF(pos->second.second);
    }
    (*map_)[hash_code] = std::pair<PyObject*, PyObject*>(k, v);

    return r;
  }

  inline PyObject* _pop(long hash_code, PyObject* v) {
    auto pos = map_->find(hash_code);
    if (pos != map_->end()) {
      Py_DECREF(pos->second.first);
      v = pos->second.second;
      map_->erase(pos);
    }
    else {
      if (v != nullptr)
        Py_INCREF(v);
    }
    return v;
  }

  inline PyObject* _popitem(void) {
    auto* r = PyTuple_New(2);
    if (r == nullptr)
      return nullptr;

    auto begin = map_->begin();
    PyTuple_SET_ITEM(r, 0, begin->second.first);
    PyTuple_SET_ITEM(r, 1, begin->second.second);
    map_->erase(begin);
    return r;
  }

  PyObject* _keys(void) const {
    auto* r = PyList_New(_size());
    if (r == nullptr)
      return nullptr;

    Py_ssize_t i = 0;
    for (auto& x : *map_) {
      auto* k = x.second.first;
      Py_INCREF(k);
      PyList_SET_ITEM(r, i++, k);
    }
    return r;
  }

  PyObject* _values(void) const {
    auto* r = PyList_New(_size());
    if (r == nullptr)
      return nullptr;

    Py_ssize_t i = 0;
    for (auto& x : *map_) {
      auto* v = x.second.second;
      Py_INCREF(v);
      PyList_SET_ITEM(r, i++, v);
    }
    return r;
  }

  PyObject* _items(void) const {
    auto* r = PyList_New(_size());
    if (r == nullptr)
      return nullptr;

    Py_ssize_t i = 0;
    for (auto& x : *map_) {
      auto* e = PyTuple_New(2);
      if (e == nullptr) {
        Py_DECREF(r);
        return nullptr;
      }
      auto* k = x.second.first;
      auto* v = x.second.second;
      Py_INCREF(k);
      Py_INCREF(v);
      PyTuple_SET_ITEM(e, 0, k);
      PyTuple_SET_ITEM(e, 1, v);

      PyList_SET_ITEM(r, i++, e);
    }
    return r;
  }

  inline void _insert(long hash_code, PyObject* k, PyObject* v) {
    auto pos = map_->find(hash_code);
    if (pos != map_->end()) {
      Py_DECREF(pos->second.first);
      Py_DECREF(pos->second.second);
    }
    (*map_)[hash_code] = std::pair<PyObject*, PyObject*>(k, v);
  }

  inline bool _delitem(long hash_code) {
    auto pos = map_->find(hash_code);
    if (pos == map_->end())
      return false;

    Py_DECREF(pos->second.first);
    Py_DECREF(pos->second.second);
    map_->erase(pos);
    return true;
  }

  inline int _traverse(visitproc visit, void* arg) {
    for (auto& x : *map_) {
      Py_VISIT(x.second.first);
      Py_VISIT(x.second.second);
    }
    return 0;
  }

  inline void _update(const nyx_dict* other) {
    for (auto& x : *other->map_) {
      auto* k = x.second.first;
      auto* v = x.second.second;
      Py_INCREF(k);
      Py_INCREF(v);
      _insert(x.first, k, v);
    }
  }

  inline IterType _get_begin(void) const {
    return map_->begin();
  }

  inline IterType _get_end(void) const {
    return map_->end();
  }
};

static bool __is_nyxdict(PyObject* o);
static PyObject* __nyxdict_new(void);

inline void __nyxdict_set_key_error(PyObject* arg) {
  auto* tup = PyTuple_Pack(1, arg);
  if (!tup)
    return;
  PyErr_SetObject(PyExc_KeyError, tup);
  Py_DECREF(tup);
}

static int _nyxdict_merge(nyx_dict* self, PyObject* b, bool is_override) {
  if (self == nullptr || !__is_nyxdict(self) || b == nullptr) {
    PyErr_BadInternalCall();
    return -1;
  }

  if (PyDict_Check(b)) {
    PyDictObject* other = reinterpret_cast<PyDictObject*>(b);
    if (self == b || other->ma_used == 0)
      return 0;

    for (auto i = 0; i <= other->ma_mask; ++i) {
      auto* entry = &other->ma_table[i];
      auto hash_code = static_cast<long>(entry->me_hash);
      if (entry->me_value != nullptr &&
          (is_override || !self->_contains(hash_code))) {
        Py_INCREF(entry->me_key);
        Py_INCREF(entry->me_value);
        self->_insert(hash_code, entry->me_key, entry->me_value);
      }
    }
  }
  else if (__is_nyxdict(b)) {
    if (self == b)
      return 0;
    self->_update(reinterpret_cast<nyx_dict*>(b));
  }
  return 0;
}

static int _nyxdict_merge_from_seq(
    nyx_dict* self, PyObject* seq, bool is_override) {
  if (self == nullptr || !__is_nyxdict(self) || seq == nullptr)
    return -1;
  auto* it = PyObject_GetIter(seq);
  if (it == nullptr)
    return -1;

  for (auto i = 0; ; ++i) {
    auto* item = PyIter_Next(it);
    if (item == nullptr) {
      if (PyErr_Occurred())
        return -1;
      break;
    }

    auto* fast = PySequence_Fast(item, "");
    if (fast == nullptr) {
      if (PyErr_ExceptionMatches(PyExc_TypeError)) {
        PyErr_Format(PyExc_TypeError,
            "cannot convert nyx_dict update sequence "
            "element #%d to a sequence", i);
      }
      Py_DECREF(item);
      return -1;
    }
    auto n = PySequence_Fast_GET_SIZE(fast);
    if (n != 2) {
      PyErr_Format(PyExc_ValueError,
          "nyx_dict update sequence element #%d has length %zd; "
          "2 is required", i, n);
      Py_DECREF(item);
      Py_DECREF(fast);
      return -1;
    }

    auto* k = PySequence_Fast_GET_ITEM(fast, 0);
    auto* v = PySequence_Fast_GET_ITEM(fast, 1);
    long hash_code;
    if (!PyString_CheckExact(k) ||
        (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
      hash_code = PyObject_Hash(k);
      if (hash_code == -1) {
        Py_DECREF(item);
        Py_DECREF(fast);
        return -1;
      }
    }
    if (is_override || !self->_contains(hash_code)) {
      Py_INCREF(k);
      Py_INCREF(v);
      self->_insert(hash_code, k, v);
    }
    Py_DECREF(item);
    Py_DECREF(fast);
  }
  return 0;
}

static int _nyxdict_update_common(
    nyx_dict* self, PyObject* args, PyObject* kwds, const char* methname) {
  PyObject* arg{};
  if (!PyArg_UnpackTuple(args, methname, 0, 1, &arg))
    return -1;

  int r{};
  if (arg != nullptr) {
    if (PyObject_HasAttrString(arg, "keys"))
      r = _nyxdict_merge(self, arg, true);
    else
      r = _nyxdict_merge_from_seq(self, arg, true);
  }
  if (r == 0 && kwds != nullptr)
    r = _nyxdict_merge(self, kwds, true);
  return r;
}

static int _nyxdict_tp_init(nyx_dict* self, PyObject* args, PyObject* kwargs) {
  self->_init();
  return _nyxdict_update_common(self, args, kwargs, "nyx_dict");
}

static void _nyxdict_tp_dealloc(nyx_dict* self) {
  self->_dealloc();
  self->ob_type->tp_free(self);
}

static PyObject* _nyxdict_tp_repr(nyx_dict* self) {
  return Py_BuildValue("s", self->_repr().c_str());
}

static int _nyxdict_tp_clear(nyx_dict* self) {
  self->_clear();
  return 0;
}

static int _nyxdict_tp_traverse(nyx_dict* self, visitproc visit, void* arg) {
  return self->_traverse(visit, arg);
}

static PyObject* _nyxdict_clear(nyx_dict* self) {
  self->_clear();
  Py_RETURN_NONE;
}

static PyObject* _nyxdict_size(nyx_dict* self) {
  return Py_BuildValue("l", self->_size());
}

static PyObject* _nyxdict_get(nyx_dict* self, PyObject* args) {
  PyObject* k{};
  PyObject* failobj = Py_None;

  if (!PyArg_UnpackTuple(args, "get", 1, 2, &k, &failobj))
    return nullptr;

  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return nullptr;
  }

  auto* v = self->_get(hash_code);
  if (v == nullptr)
    v = failobj;
  Py_INCREF(v);
  return v;
}

static PyObject* _nyxdict_setdefault(nyx_dict* self, PyObject* args) {
  PyObject* k{};
  PyObject* failobj = Py_None;

  if (!PyArg_UnpackTuple(args, "setdefault", 1, 2, &k, &failobj))
    return nullptr;
  if (self == failobj) {
    PyErr_SetString(PyExc_SystemError,
        "setdefault(key, value): cannot set default as self");
    return nullptr;
  }

  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return nullptr;
  }

  auto* v = self->_setdefault(hash_code, k, failobj);
  Py_INCREF(v);
  return v;
}

static PyObject* _nyxdict_pop(nyx_dict* self, PyObject* args) {
  PyObject* k{};
  PyObject* failobj{};

  if (!PyArg_UnpackTuple(args, "pop", 1, 2, &k, &failobj))
    return nullptr;

  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return nullptr;
  }

  auto* v = self->_pop(hash_code, failobj);
  if (v == nullptr) {
    __nyxdict_set_key_error(k);
    return nullptr;
  }
  return v;
}

static PyObject* _nyxdict_popitem(nyx_dict* self) {
  if (self->_size() == 0) {
    PyErr_SetString(PyExc_KeyError, "popitem(): nyx_dict is empty");
    return nullptr;
  }

  return self->_popitem();
}

static PyObject* _nyxdict_keys(nyx_dict* self) {
  return self->_keys();
}

static PyObject* _nyxdict_values(nyx_dict* self) {
  return self->_values();
}

static PyObject* _nyxdict_items(nyx_dict* self) {
  return self->_items();
}

static PyObject* _nyxdict_contains(nyx_dict* self, PyObject* k) {
  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return nullptr;
  }

  return PyBool_FromLong(static_cast<long>(self->_contains(hash_code)));
}

static PyObject* _nyxdict_update(
    nyx_dict* self, PyObject* args, PyObject* kwds) {
  if (_nyxdict_update_common(self, args, kwds, "update") == -1)
    return nullptr;
  Py_RETURN_NONE;
}

static PyObject* _nyxdict_copy(nyx_dict* self) {
  if (self == nullptr || !__is_nyxdict(self)) {
    PyErr_BadInternalCall();
    return nullptr;
  }

  auto* r = static_cast<nyx_dict*>(__nyxdict_new());
  if (r == nullptr)
    return nullptr;
  if (_nyxdict_merge(r, self, true) == -1) {
    Py_DECREF(r);
    return nullptr;
  }
  return r;
}

static int _nyxdict__meth_contains(nyx_dict* self, PyObject* k) {
  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return -1;
  }

  return static_cast<int>(self->_contains(hash_code));
}

static Py_ssize_t _nyxdict__meth_length(nyx_dict* self) {
  return self->_size();
}

static PyObject* _nyxdict__meth_subscript(nyx_dict* self, PyObject* k) {
  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return nullptr;
  }

  auto* v = self->_get(hash_code);
  if (v == nullptr) {
    __nyxdict_set_key_error(k);
    return nullptr;
  }
  Py_INCREF(v);
  return v;
}

static int _nyxdict_setitem(nyx_dict* self, PyObject* k, PyObject* v) {
  if (self == v) {
    PyErr_SetString(PyExc_SystemError,
        "__setitem__(key, value): cannot set item as self");
    return -1;
  }
  if (!__is_nyxdict(self)) {
    PyErr_BadInternalCall();
    return -1;
  }

  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return -1;
  }

  Py_INCREF(k);
  Py_INCREF(v);
  self->_insert(hash_code, k, v);
  return 0;
}

static int _nyxdict_delitem(nyx_dict* self, PyObject* k) {
  if (!__is_nyxdict(self)) {
    PyErr_BadInternalCall();
    return -1;
  }

  long hash_code;
  if (!PyString_CheckExact(k) ||
      (hash_code = ((PyStringObject*)k)->ob_shash) == -1) {
    hash_code = PyObject_Hash(k);
    if (hash_code == -1)
      return -1;
  }

  if (!self->_delitem(hash_code)) {
    __nyxdict_set_key_error(k);
    return -1;
  }
  return 0;
}

static int
_nyxdict__meth_ass_subscript(nyx_dict* self, PyObject* k, PyObject* v) {
  if (v == nullptr)
    return _nyxdict_delitem(self, k);
  else
    return _nyxdict_setitem(self, k, v);
}

struct nyx_dictiter : public PyObject {
  using IterType =
    std::unordered_map<long, std::pair<PyObject*, PyObject*>>::const_iterator;
  nyx_dict* di_dict{};
  PyObject* di_result{};
  IterType* di_iter{};
};

static void _nyxdictiter_dealloc(nyx_dictiter* di) {
  Py_XDECREF(di->di_dict);
  Py_XDECREF(di->di_result);
  if (di->di_iter != nullptr)
    delete di->di_iter;
  PyObject_GC_Del(di);
}

static int _nyxdictiter_traverse(nyx_dictiter* di, visitproc visit, void* arg) {
  Py_VISIT(di->di_dict);
  Py_VISIT(di->di_result);
  return 0;
}

static PyObject* _nyxdictiter_len(nyx_dictiter* di) {
  Py_ssize_t len = 0;
  if (di->di_dict != nullptr)
    len = di->di_dict->_size();
  return PyInt_FromSsize_t(len);
}

PyDoc_STRVAR(__nyxdictiter_hint_doc,
"private method returning an estimate of len(list(it))");
static PyMethodDef _nyxdictiter_methods[] = {
  {"__length_hint__", (PyCFunction)_nyxdictiter_len, METH_NOARGS, __nyxdictiter_hint_doc},
  {nullptr, nullptr}
};

static inline
std::tuple<bool, PyObject*, PyObject*> _nyxdictiter_iternext(nyx_dictiter* di) {
  auto* d = di->di_dict;
  if (d == nullptr || !__is_nyxdict(d))
    return std::make_tuple(false, nullptr, nullptr);
  if (*di->di_iter == d->_get_end()) {
    Py_DECREF(d);
    di->di_dict = nullptr;
    return std::make_tuple(false, nullptr, nullptr);
  }

  auto* k = (*di->di_iter)->second.first;
  auto* v = (*di->di_iter)->second.second;
  ++(*di->di_iter);
  return std::make_tuple(true, k, v);
}

static PyObject* _nyxdictiter_iternextkey(nyx_dictiter* di) {
  auto r = _nyxdictiter_iternext(di);
  if (!std::get<0>(r))
    return nullptr;

  auto* k = std::get<1>(r);
  Py_INCREF(k);
  return k;
}

PyTypeObject _nyxdictiter_keytype = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "nyx_dict-keyiter", // tp_name
  sizeof(nyx_dictiter), // tp_basicsize
  0, // tp_itemsize
  (destructor)_nyxdictiter_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  0, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, // tp_flags
  0, // tp_doc
  (traverseproc)_nyxdictiter_traverse, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  PyObject_SelfIter, // tp_iter
  (iternextfunc)_nyxdictiter_iternextkey, // tp_iternext
  _nyxdictiter_methods, // tp_methods
  0,
};

static PyObject* _nyxdictiter_iternextvalue(nyx_dictiter* di) {
  auto r = _nyxdictiter_iternext(di);
  if (!std::get<0>(r))
    return nullptr;

  auto* v = std::get<2>(r);
  Py_INCREF(v);
  return v;
}

PyTypeObject _nyxdictiter_valuetype = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "nyx_dict-valueiter", // tp_name
  sizeof(nyx_dictiter), // tp_basicsize
  0, // tp_itemsize
  (destructor)_nyxdictiter_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  0, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, // tp_flags
  0, // tp_doc
  (traverseproc)_nyxdictiter_traverse, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  PyObject_SelfIter, // tp_iter
  (iternextfunc)_nyxdictiter_iternextvalue, // tp_iternext
  _nyxdictiter_methods, // tp_methods
  0,
};

static PyObject* _nyxdictiter_iternextitem(nyx_dictiter* di) {
  auto* result = di->di_result;
  if (result->ob_refcnt == 1) {
    Py_INCREF(result);
    Py_DECREF(PyTuple_GET_ITEM(result, 0));
    Py_DECREF(PyTuple_GET_ITEM(result, 1));
  }
  else {
    result = PyTuple_New(2);
    if (result == nullptr)
      return nullptr;
  }
  auto r = _nyxdictiter_iternext(di);
  if (!std::get<0>(r))
    return nullptr;

  auto* k = std::get<1>(r);
  auto* v = std::get<2>(r);
  Py_INCREF(k);
  Py_INCREF(v);

  PyTuple_SET_ITEM(result, 0, k);
  PyTuple_SET_ITEM(result, 1, v);
  return result;
}

PyTypeObject _nyxdictiter_itemtype = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "nyx_dict-itemiter", // tp_name
  sizeof(nyx_dictiter), // tp_basicsize
  0, // tp_itemsize
  (destructor)_nyxdictiter_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  0, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, // tp_flags
  0, // tp_doc
  (traverseproc)_nyxdictiter_traverse, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  PyObject_SelfIter, // tp_iter
  (iternextfunc)_nyxdictiter_iternextitem, // tp_iternext
  _nyxdictiter_methods, // tp_methods
  0,
};

static PyObject* _nyxdictiter_new(nyx_dict* dict, PyTypeObject* itertype) {
  auto* di = PyObject_GC_New(nyx_dictiter, itertype);
  if (di == nullptr)
    return nullptr;
  Py_INCREF(dict);
  di->di_dict = dict;
  di->di_result = nullptr;
  if (itertype == &_nyxdictiter_itemtype) {
    di->di_result = PyTuple_Pack(2, Py_None, Py_None);
    if (di->di_result == nullptr) {
      Py_DECREF(di);
      return nullptr;
    }
  }
  di->di_iter = new nyx_dictiter::IterType();
  *di->di_iter = dict->_get_begin();

  _PyObject_GC_TRACK(di);
  return static_cast<PyObject*>(di);
}

static PyObject* _nyxdict_iterkeys(nyx_dict* self) {
  return _nyxdictiter_new(self, &_nyxdictiter_keytype);
}

static PyObject* _nyxdict_itervalues(nyx_dict* self) {
  return _nyxdictiter_new(self, &_nyxdictiter_valuetype);
}

static PyObject* _nyxdict_iteritems(nyx_dict* self) {
  return _nyxdictiter_new(self, &_nyxdictiter_itemtype);
}

PyDoc_STRVAR(_nyxdict_doc,
"nyx_dict() -> new empty nyx_dict\n"
"nyx_dict(mapping) -> new nyx_dict initialized from a mapping object's\n"
"        (key, value) pairs\n"
"nyx_dict(iterable) -> new nyx_dict initialized as if via:\n"
"        d = {}\n"
"        for k, v in iterable:\n"
"            d[k] = v\n"
"nyx_dict(**kwargs) -> new nyx_dict initialized with the name=value pairs\n"
"        in the keyword argument list. For example:  nyx_dict(one=1, two=2)");
PyDoc_STRVAR(__nyxdict_clear_doc,
"D.clear() -> None -- remove all items from D");
PyDoc_STRVAR(__nyxdict_size_doc,
"D.size() -> iteger -- return number of items in D");
PyDoc_STRVAR(__nyxdict_haskey_doc,
"D.hash_key(k) -> boolean -- return True if D has a key k, else False");
PyDoc_STRVAR(__nyxdict_get_doc,
"D.get(k[, d]) -- return D[k] if k in D, else d. d defaults to None");
PyDoc_STRVAR(__nyxdict_setdefault_doc,
"D.setdefault(k[, d]) -- return D.get(k, d), also set D[k] = d if k not in D");
PyDoc_STRVAR(__nyxdict_pop_doc,
"D.pop(k[, d]) -- remove specified key and return the corresponding value.\n"
"If key is not found, d is returned if given, otherwise KeyError is raised");
PyDoc_STRVAR(__nyxdict_popitem_doc,
"D.popitem() -> tuple -- remove and return some (key, value) pair as a\n"
"2-tuple; but raise KeyError if D is empty.");
PyDoc_STRVAR(__nyxdict_keys_doc,
"D.keys() -> list -- list of D's keys");
PyDoc_STRVAR(__nyxdict_values_doc,
"D.values() -> list -- list of D's values");
PyDoc_STRVAR(__nyxdict_items_doc,
"D.items() -> list -- list of D's (key, value) pairs, as 2-tuples");
PyDoc_STRVAR(__nyxdict_update_doc,
"D.update([E, ]**F) -> None -- update D from dict/iterable E and F.\n"
"if E present and has a .keys() method, does: for k in E: D[k] = E[k]\n"
"if E present and lacks .keys() methos, does: for (k, v) in E: D[k] = v\n"
"in either case, this is followed by: for k in F: D[k] = F[k]");
PyDoc_STRVAR(__nyxdict_copy_doc,
"D.copy() -> nyx_dict -- a shallow copy of D");
PyDoc_STRVAR(__nyxdict_iterkeys_doc,
"D.iterkeys() -- an iterator over the keys of D");
PyDoc_STRVAR(__nyxdict_itervalues_doc,
"D.itervalues() -- an iterator over the values of D");
PyDoc_STRVAR(__nyxdict_iteritems_doc,
"D.iteritems() -- an iterator over the (key, value) items of D");
PyDoc_STRVAR(__nyxdict_contains_doc,
"D.__contains__(k) -> boolean -- return True if D has a key k, else False");
PyDoc_STRVAR(__nyxdict_getitem_doc,
"x.__getitem__(y) <==> x[y]");

static PySequenceMethods _nyxdict_as_sequence = {
  0, // sq_length
  0, // sq_concat
  0, // sq_repeat
  0, // sq_item
  0, // sq_slice
  0, // sq_ass_item
  0, // sq_ass_slice
  (objobjproc)_nyxdict__meth_contains, // sq_contains
  0, // sq_inplace_concat
  0, // sq_inplace_repeat
};

static PyMappingMethods _nyxdict_as_mapping = {
  (lenfunc)_nyxdict__meth_length, // mp_length
  (binaryfunc)_nyxdict__meth_subscript, // mp_subscript
  (objobjargproc)_nyxdict__meth_ass_subscript, // mp_ass_subscript
};

static PyMethodDef _nyxdict_methods[] = {
  {"clear", (PyCFunction)_nyxdict_clear, METH_NOARGS, __nyxdict_clear_doc},
  {"size", (PyCFunction)_nyxdict_size, METH_NOARGS, __nyxdict_size_doc},
  {"hash_key", (PyCFunction)_nyxdict_contains, METH_O | METH_COEXIST, __nyxdict_haskey_doc},
  {"get", (PyCFunction)_nyxdict_get, METH_VARARGS, __nyxdict_get_doc},
  {"setdefault", (PyCFunction)_nyxdict_setdefault, METH_VARARGS, __nyxdict_setdefault_doc},
  {"pop", (PyCFunction)_nyxdict_pop, METH_VARARGS, __nyxdict_pop_doc},
  {"popitem", (PyCFunction)_nyxdict_popitem, METH_NOARGS, __nyxdict_popitem_doc},
  {"keys", (PyCFunction)_nyxdict_keys, METH_NOARGS, __nyxdict_keys_doc},
  {"values", (PyCFunction)_nyxdict_values, METH_NOARGS, __nyxdict_values_doc},
  {"items", (PyCFunction)_nyxdict_items, METH_NOARGS, __nyxdict_items_doc},
  {"update", (PyCFunction)_nyxdict_update, METH_VARARGS | METH_KEYWORDS, __nyxdict_update_doc},
  {"copy", (PyCFunction)_nyxdict_copy, METH_NOARGS, __nyxdict_copy_doc},
  {"iterkeys", (PyCFunction)_nyxdict_iterkeys, METH_NOARGS, __nyxdict_iterkeys_doc},
  {"itervalues", (PyCFunction)_nyxdict_itervalues, METH_NOARGS, __nyxdict_itervalues_doc},
  {"iteritems", (PyCFunction)_nyxdict_iteritems, METH_NOARGS, __nyxdict_iteritems_doc},
  {"__contains__", (PyCFunction)_nyxdict_contains, METH_O | METH_COEXIST, __nyxdict_contains_doc},
  {"__getitem__", (PyCFunction)_nyxdict__meth_subscript, METH_O | METH_COEXIST, __nyxdict_getitem_doc},
  {nullptr}
};

static PyTypeObject _nyxdict_type = {
  PyObject_HEAD_INIT(nullptr)
  0, // ob_size
  "_nyxcore.nyx_dict", // tp_name
  sizeof(nyx_dict), // tp_basesize
  0, // tp_itemsize
  (destructor)_nyxdict_tp_dealloc, // tp_dealloc
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_compare
  (reprfunc)_nyxdict_tp_repr, // tp_repr
  0, // tp_as_number
  &_nyxdict_as_sequence, // tp_as_sequence
  &_nyxdict_as_mapping, // tp_as_mapping
  (hashfunc)PyObject_HashNotImplemented, // tp_hash
  0, // tp_call
  (reprfunc)_nyxdict_tp_repr, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE, // tp_flags
  _nyxdict_doc, // tp_doc
  (traverseproc)_nyxdict_tp_traverse, // tp_traverse
  (inquiry)_nyxdict_tp_clear, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  _nyxdict_methods, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  (initproc)_nyxdict_tp_init, // tp_init
  PyType_GenericAlloc, // tp_alloc
  PyType_GenericNew, // tp_new
  PyObject_GC_Del, // tp_free
};

bool __is_nyxdict(PyObject* o) {
  return Py_TYPE(o) == &_nyxdict_type;
}

PyObject* __nyxdict_new(void) {
  PyTypeObject* _type = &_nyxdict_type;
  auto* nd = (nyx_dict*)_type->tp_alloc(_type, 0);
  if (nd != nullptr)
    nd->_init();
  return nd;
}

void nyx_dict_wrap(PyObject* m) {
  if (PyType_Ready(&_nyxdict_type) < 0)
    return;

  auto* _type = reinterpret_cast<PyObject*>(&_nyxdict_type);
  Py_INCREF(_type);
  PyModule_AddObject(m, "nyx_dict", _type);
}

}
