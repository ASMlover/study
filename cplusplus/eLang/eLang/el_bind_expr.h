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
#ifndef __EL_BIND_EXPR_HEADER_H__
#define __EL_BIND_EXPR_HEADER_H__

#include "el_expr_compiler_base.h"
#include "el_expr.h"
#include "el_define_expr.h"

namespace el {

class BindExpr : public DefineExpr {
  Ref<Expr> target_;
public:
  BindExpr(Ref<Expr> target)
    : target_(target) {
  }

  inline Ref<Expr> Target(void) const {
    return target_;
  }

  virtual void Trace(std::ostream& stream) const override {
    stream << target_ << " :: ";
    if (Definitions().Count() == 1) {
      stream << Definitions()[0].GetName()
             << " " << Definitions()[0].GetBody();
    }
    else {
      stream << "( ";
      for (auto i = 0; i < Definitions().Count(); ++i) {
        stream << Definitions()[0].GetName() << " "
               << Definitions()[0].GetBody();
      }
      stream << " )";
    }
  }

  EL_EXPR_VISITOR
};

}

#endif  // __EL_BIND_EXPR_HEADER_H__
