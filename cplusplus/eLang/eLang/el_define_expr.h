// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_DEFINE_EXPR_HEADER_H__
#define __EL_DEFINE_EXPR_HEADER_H__

#include "el_expr.h"
#include "el_expr_compiler_base.h"

namespace el {

class Definition {
  bool      is_method_;
  String    name_;
  Ref<Expr> body_;
public:
  Definition(void)
    : is_method_(false)
    , name_()
    , body_() {
  }

  Definition(bool is_method, const String& name, const Ref<Expr>& body)
    : is_method_(is_method)
    , name_(name)
    , body_(body) {
  }

  Definition& operator=(const Definition& other) {
    if (&other != this) {
      is_method_ = other.is_method_;
      name_      = other.name_;
      body_      = other.body_;
    }

    return *this;
  }

  inline bool IsMethod(void) const {
    return is_method_;
  }

  inline String GetName(void) const {
    return name_;
  }

  inline const Ref<Expr>& GetBody(void) const {
    return body_;
  }
};

class DefineExpr : public Expr {
  Array<Definition> definitions_;
public:
  DefineExpr(void) {
  }

  inline const Array<Definition>& Definitions(void) const {
    return definitions_;
  }

  inline void Define(
      bool is_method, const String& name, const Ref<Expr>& body) {
    definitions_.Append(Definition(is_method, name, body));
  }
};

}

#endif  // __EL_DEFINE_EXPR_HEADER_H__
