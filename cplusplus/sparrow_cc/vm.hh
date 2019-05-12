// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <list>
#include <vector>
#include <memory>
#include "common.hh"
#include "lexer.hh"
#include "base_object.hh"

namespace sparrow {

enum class VMResult {
  SUCCESS,
  ERROR,
};

class VM : private UnCopyable {
  static constexpr sz_t kGCThresholds = 1 << 20;

  ClassObject* clscls_{}; // class
  ClassObject* objcls_{}; // object
  ClassObject* strcls_{}; // string
  ClassObject* mapcls_{}; // map
  ClassObject* rngcls_{}; // range
  ClassObject* listcls_{}; // list
  ClassObject* nilcls_{}; // nil
  ClassObject* boolcls_{}; // boolean
  ClassObject* numcls_{}; // numeric
  ClassObject* funcls_{}; // function
  ClassObject* thrdcls_{}; // thread

  sz_t bytes_allocated_{};
  sz_t next_gc_{kGCThresholds};
  std::list<BaseObject*> all_objects_;

  std::vector<str_t> all_method_names_;
  MapObject* all_modules_{};
  ThreadObject* curr_thrd_{};

  std::shared_ptr<Lexer> lex_;
public:
  VM(void);
  ~VM(void);

  inline ClassObject* clscls(void) const { return clscls_; }
  inline ClassObject* objcls(void) const { return objcls_; }
  inline ClassObject* strcls(void) const { return strcls_; }
  inline ClassObject* mapcls(void) const { return mapcls_; }
  inline ClassObject* rngcls(void) const { return rngcls_; }
  inline ClassObject* listcls(void) const { return listcls_; }
  inline ClassObject* nilcls(void) const { return nilcls_; }
  inline ClassObject* boolcls(void) const { return boolcls_; }
  inline ClassObject* numcls(void) const { return numcls_; }
  inline ClassObject* funcls(void) const { return funcls_; }
  inline ClassObject* thrdcls(void) const { return thrdcls_; }

  void append_object(BaseObject* obj);
  void set_module(const Value& name, const Value& module);
};

}
