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

#include <iostream>
#include <map>
#include <boost/python.hpp>
namespace py = ::boost::python;

struct ObjNode {
  int obj_id{};

  ObjNode(int id)
    : obj_id(id) {
  }

  virtual ~ObjNode(void) {
    std::cout << "ObjNode<" << this << ">::~ObjNode" << std::endl;
  }
};

class ObjNodeWrap : public ObjNode {
  PyObject* self_{};
public:
  ObjNodeWrap(PyObject* self, int id = 0)
    : ObjNode(id)
    , self_(self) {
    py::xincref(self_);
  }

  virtual ~ObjNodeWrap(void) {
    std::cout << "ObjNodeWrap<" << this << ">::~ObjNodeWrap" << std::endl;
  }

  void show_default(void) {
    std::cout << "ObjNodeWrap<" << this << ">::show" << std::endl;
  }
};

class ObjManager : private boost::noncopyable {
  using ObjNodePtr = boost::shared_ptr<ObjNode>;
  std::map<int, ObjNodePtr> objdict_;

  ObjManager(void) {}

  ~ObjManager(void) {
    objdict_.clear();
  }
public:
  static ObjManager& instance(void) {
    static ObjManager ins;
    return ins;
  }

  void add_object(const ObjNodePtr& o) {
    auto it = objdict_.find(o->obj_id);
    if (it != objdict_.end()) {
      std::cout
        << "ObjManager<" << this << "> "
        << o->obj_id << " object exists" << std::endl;
    }
    else {
      objdict_[o->obj_id] = o;
    }
  }
};

#define OBJMGR ObjManager::instance
