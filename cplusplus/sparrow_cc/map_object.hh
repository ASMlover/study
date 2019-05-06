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

#include <optional>
#include <unordered_map>
#include "base_object.hh"
#include "value.hh"

namespace sparrow {

struct KeyHash {
  inline sz_t operator()(const Value& k) const {
    return k.hasher();
  }
};

struct KeyEqual {
  inline bool operator()(const Value& lhs, const Value& rhs) const {
    if (lhs.hasher() == rhs.hasher())
      return true;
    return lhs == rhs;
  }
};

class MapObject : public BaseObject {
  std::unordered_map<Value, Value, KeyHash, KeyEqual> entries_;

  MapObject(VM& vm);
  virtual ~MapObject(void);
public:
  void set(const Value& key, const Value& val);
  std::optional<Value> get(const Value& key) const;
  Value remove(const Value& key);
  void clear(void);

  static MapObject* create(VM& vm) {
    return new MapObject(vm);
  }
};

}
