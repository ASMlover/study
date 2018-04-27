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
#include "nyx_safe_container.h"

namespace nyx { namespace safe {

SafeIterDict::SafeIterDict(void)
  : first_() {
}

SafeIterDict::~SafeIterDict(void) {
  clear();
}

PyObject* SafeIterDict::get(int key, PyObject* d) const {
  auto pos = items_.find(key);
  if (pos == items_.end())
    return d;
  return pos->second->value;
}

PyObject* SafeIterDict::insert(int key, PyObject* value, bool is_override) {
  auto pos = items_.find(key);
  if (pos == items_.end()) {
    auto item = std::make_shared<SafeIterItem>(key, value, SafeIterNodePtr());
    ++gen_node_id_;
    auto node = std::make_shared<SafeIterNode>(
        SafeIterNodePtr(), SafeIterNodePtr(), item, true, gen_node_id_);
    item->node = node;
    items_[key] = item;
    if (first_) {
      node->next = first_;
      node->prev = first_->prev;
      node->prev->next = node;
      first_->prev = node;
    }
    else {
      first_ = node;
      node->next = node;
      node->prev = node;
    }
  }
  else {
    if (is_override) {
      Py_INCREF(value);
      Py_DECREF(pos->second->value);
      pos->second->value = value;
    }
    else {
      return pos->second->value;
    }
  }

  return value;
}

SafeIterItemPtr SafeIterDict::pop(int key) {
  auto pos = items_.find(key);
  if (pos == items_.end())
    return SafeIterItemPtr();

  auto item = pos->second;
  items_.erase(pos);
  auto node = item->node;
  item->node.reset();
  node->item.reset();
  node->is_valid = false;
  if (node->next == node) {
    first_.reset();
    node->next.reset();
    node->prev.reset();
  }
  else {
    if (node == first_)
      first_ = node->next;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->prev.reset();
  }
  return item;
}

void SafeIterDict::clear(void) {
  if (first_) {
    items_.clear();
    auto node = first_;
    do {
      node->item->node.reset();
      node->item.reset();
      node->prev->next.reset();
      node->prev.reset();
      node->is_valid = false;
      node = node->next;
    } while (node);
    first_.reset();
  }
}

void SafeIterDict::begin(SafeIterDictIter* iter, bool duplicate) {
  iter->init(this, duplicate);
}

SafeIterDictIter::SafeIterDictIter(void) {
}

SafeIterDictIter::~SafeIterDictIter(void) {
  clear();
}

void SafeIterDictIter::init(SafeIterDict* dict, bool duplicate) {
  if (dict->first_) {
    if (is_valid_)
      clear();

    dict_ = dict;
    iter_ = dict->first_;
    is_valid_ = true;
    if (!duplicate)
      iter_cache_ = new std::unordered_set<int>(dict->size() + 5);
    else
      iter_cache_ = nullptr;
  }
}

void SafeIterDictIter::clear(void) {
  is_valid_ = false;
  iter_.reset();
  dict_ = nullptr;
  if (iter_cache_ != nullptr) {
    delete iter_cache_;
    iter_cache_ = nullptr;
  }
}

void SafeIterDictIter::next(void) {
  if (!is_valid_)
    return;

  do {
    if (iter_->next && iter_->node_id >= iter_->next->node_id) {
      is_valid_ = false;
      return;
    }
    if (iter_cache_ != nullptr && iter_->is_valid)
      iter_cache_->insert(iter_->item->key);
    iter_ = iter_->next;
    if (!iter_) {
      if (dict_->first_) {
        iter_ = dict_->first_;
      }
      else {
        is_valid_ = false;
        return;
      }
    }
  } while (!iter_->is_valid || (iter_cache_ != nullptr &&
        iter_cache_->find(iter_->item->key) != iter_cache_->end()));
}

bool SafeIterDictIter::is_valid(void) {
  if (is_valid_ && !iter_->is_valid)
    next();
  return is_valid_;
}

typedef struct _safeiterdictobject {
  PyObject_HEAD
  SafeIterDict* tb_table;
} PySafeIterDictObject;

#if !defined(PySafeIterDict_MAXFREELIST)
# define PySafeIterDict_MAXFREELIST 80
#endif
static PySafeIterDictObject* _free_list[PySafeIterDict_MAXFREELIST];
static int _numfree = 0;

static PyObject* dict_tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds);
static void dict_tp_dealloc(register PySafeIterDictObject* mp);

PyDoc_STRVAR(dictionary_doc,
"SafeIterDict() -> new empty dictionary\n"
"SafeIterDict(mapping) -> new dictionary initialized from a mapping object's\n"
"   (key, value) pairs\n"
"SafeIterDict(iterable) -> new dictionary initialized as if via:\n"
"   d = SafeIterDict()\n"
"   for k, v in iterable:\n"
"     d[k] = v\n"
"SafeIterDict(**kwargs) -> new dictionary initialized with the name=value pairs\n"
"   in the keyword argument list. For example: SafeIterDict(one=1, two=2)");

static PyTypeObject _safeiterdict_type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "SafeIterDict", // tp_name
  sizeof(PySafeIterDictObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)dict_tp_dealloc, // tp_dealloc
  (printfunc)0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  (cmpfunc)0, // tp_compare
  (reprfunc)0, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  (hashfunc)PyObject_HashNotImplemented, // tp_hash
  0, // tp_call
  (reprfunc)0, // tp_str
  PyObject_GenericGetAttr, // tp_getattro
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
    Py_TPFLAGS_BASETYPE | Py_TPFLAGS_DICT_SUBCLASS, // tp_flags
  dictionary_doc, // tp_doc
  (traverseproc)0, // tp_traverse
  (inquiry)0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  (getiterfunc)0, // tp_iter
  0, // tp_iternext
  0, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_desct_set
  0, // tp_dictoffset
  (initproc)0, // tp_init
  PyType_GenericAlloc, // tp_alloc
  dict_tp_new, // tp_new
  PyObject_GC_Del, // tp_free
};

#define PySafeIterDict_CheckExact(op) (Py_TYPE(op) == &_safeiterdict_type)

PyObject* dict_tp_new(
    PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/) {
  register PySafeIterDictObject* mp;
  if (_numfree) {
    mp = _free_list[--_numfree];
    _Py_NewReference((PyObject*)mp);
  }
  else {
    mp = PyObject_GC_New(PySafeIterDictObject, &_safeiterdict_type);
    if (mp == nullptr)
      return nullptr;
    mp->tb_table = new SafeIterDict();
  }
  return (PyObject*)mp;
}

void dict_tp_dealloc(register PySafeIterDictObject* mp) {
  auto* tb = mp->tb_table;
  PyObject_GC_UnTrack(mp);
  Py_TRASHCAN_SAFE_BEGIN(mp)
  tb->clear();
  if (_numfree < PySafeIterDict_MAXFREELIST && PySafeIterDict_CheckExact(mp))
    _free_list[_numfree++] = mp;
  else
    Py_TYPE(mp)->tp_free((PyObject*)mp);
  Py_TRASHCAN_SAFE_END(mp)
}

void nyx_safeiterdict_wrap(PyObject* m) {
  if (PyType_Ready(&_safeiterdict_type) < 0)
    return;

  auto* _type = reinterpret_cast<PyObject*>(&_safeiterdict_type);
  Py_INCREF(_type);
  PyModule_AddObject(m, "SafeIterDict", _type);
}

}}
