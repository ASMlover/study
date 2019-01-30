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

#include <string>

namespace lox {

enum class ObjType {
  STRING
};

class StringObject;

class Object {
  ObjType type_;
public:
  Object(ObjType t);
  virtual ~Object(void) {}

  ObjType get_type(void) const { return type_; }
  bool is_string(void) const { return type_ == ObjType::STRING; }

  virtual bool is_equal(const Object* r) const { return this == r; }
  virtual bool is_truthy(void) const { return false; }
  virtual std::string stringify(void) const { return "object"; }

  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
};

StringObject* concat(const Object* x, const Object* y);

class StringObject : public Object {
  int length_{};
  char* chars_{};
public:
  StringObject(void) : Object(ObjType::STRING) {}
  StringObject(const std::string& s);
  ~StringObject(void);

  StringObject(const StringObject& s);
  StringObject(StringObject&& s);
  StringObject& operator=(const StringObject& s);
  StringObject& operator=(StringObject&& s);

  int length(void) const { return length_; }
  const char* data(void) const { return chars_; }
  const char* c_str(void) const { return chars_; }
  void reset(const char* s, int n);

  virtual bool is_equal(const Object* r) const override;
  virtual bool is_truthy(void) const override;
  virtual std::string stringify(void) const override;
};

}
