// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __VALUE_HEADER_H__
#define __VALUE_HEADER_H__

enum class ValueType {
  VALUETYPE_INVAL = 0,
  VALUETYPE_BOOL  = 3,
  VALUETYPE_INT   = 4,
  VALUETYPE_REAL  = 5,
  VALUETYPE_STR   = 6,
  VALUETYPE_UNDEF = 7,
};

class Value {
  typedef std::string* StringPtr;
public:
  ValueType type_;
  uint32_t  next_;
  union {
    bool  bool_;
    int   int_;
    float real_;
    StringPtr string_;
  };
public:
  Value(void);
  explicit Value(const Value& v);
  explicit Value(bool v);
  explicit Value(int v);
  explicit Value(float v);
  explicit Value(const std::string& v);

  virtual ~Value(void);

  Value& operator=(const Value& v);

  const std::string ToString(void);
  int GetInteger(void);
  float GetReal(void);

  void Add(Value& v);
  void Sub(Value& v);
  void Mul(Value& v);
  void Div(Value& v);
  void Mod(Value& v);
  void Pow(Value& v);

  bool Is(void) const;
  bool IsNot(void) const;
  bool Lt(Value& v);
  bool Gt(Value& v);
  bool Le(Value& v);
  bool Ge(Value& v);
  bool Eq(Value& v);
  bool Ne(Value& v);
};

#endif  // __VALUE_HEADER_H__
