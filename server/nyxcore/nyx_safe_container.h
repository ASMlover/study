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
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <Python.h>

namespace nyx { namespace safe {

void nyx_safeiterdict_wrap(PyObject* m);

struct SafeIterNode;
struct SafeIterItem;
class SafeIterDictIter;

using SafeIterNodePtr = std::shared_ptr<SafeIterNode>;
using SafeIterItemPtr = std::shared_ptr<SafeIterItem>;

struct SafeIterNode {
  SafeIterNodePtr next;
  SafeIterNodePtr prev;
  SafeIterItemPtr item;
  bool is_valid{};
  std::size_t node_id{};

  SafeIterNode(const SafeIterNodePtr& _next,
      const SafeIterNodePtr& _prev,
      const SafeIterItemPtr& _item,
      bool valid, std::size_t id)
    : next(_next)
    , prev(_prev)
    , item(_item)
    , is_valid(valid)
    , node_id(id) {
  }
};

struct SafeIterItem {
  int key{};
  PyObject* value{};
  SafeIterNodePtr node;

  SafeIterItem(int k, PyObject* v, const SafeIterNodePtr& n)
    : key(k)
    , value(v)
    , node(n) {
    Py_INCREF(value);
  }

  ~SafeIterItem(void) {
    Py_DECREF(value);
  }
};

class SafeIterDict {
  std::unordered_map<int, SafeIterItemPtr> items_;
  SafeIterNodePtr first_;
  std::size_t gen_node_id_{};

  friend class SafeIterDictIter;
public:
  SafeIterDict(void);
  ~SafeIterDict(void);

  PyObject* get(int key, PyObject* d = Py_None) const;
  PyObject* insert(int key, PyObject* value, bool is_override);
  SafeIterItemPtr pop(int key);
  void clear(void);
  void begin(SafeIterDictIter* iter, bool duplicate = false);

  PyObject* setdefault(int key, PyObject* d = Py_None) {
    return insert(key, d, false);
  }

  void setitem(int key, PyObject* value) {
    insert(key, value, true);
  }

  void delitem(int key) {
    pop(key);
  }

  bool has_key(int key) const {
    return items_.find(key) != items_.end();
  }

  Py_ssize_t size(void) const {
    return static_cast<Py_ssize_t>(items_.size());
  }
};

class SafeIterDictIter {
  SafeIterDict* dict_{};
  SafeIterNodePtr iter_;
  bool is_valid_{};
  std::unordered_set<int>* iter_cache_{};
public:
  SafeIterDictIter(void);
  ~SafeIterDictIter(void);

  void init(SafeIterDict* dict, bool duplicate);
  void clear(void);

  void next(void);
  bool is_valid(void);

  SafeIterItemPtr get(void) const {
    return iter_->item;
  }
};

}}
