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

#include <functional>

namespace pysmall {

struct TypeObject;

struct Object {
  int refcount{};
  TypeObject* type{};

  Object(int rc = 1) : refcount(rc) {}
  virtual ~Object(void) {}
};

struct TypeObject : public Object {
  using printfunc = std::function<void (Object*)>;
  using plusfunc = std::function<Object* (Object*, Object*)>;
  using hashfunc = std::function<long (Object*)>;

  const char* name{};
  printfunc print_fn{};
  plusfunc plus_fn{};
  hashfunc hash_fn{};

  TypeObject(int rc = 0, const char* n = "type")
    : Object(rc)
    , name(n) {
  }

  void set_printfunc(printfunc&& fn) { print_fn = std::move(fn); }
  void set_plusfunc(plusfunc&& fn) { plus_fn = std::move(fn); }
  void set_hashfunc(hashfunc&& fn) { hash_fn = std::move(fn); }
};

}
