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
#ifndef __EL_EXPR_HEADER_H__
#define __EL_EXPR_HEADER_H__

#define EL_EXPR_VISITOR\
virtual void Accept(ExprCompilerBase& compiler, int dest) const override {\
  compiler.Visit(*this, dest);\
}

namespace el {

interface ExprCompilerBase;
class Object;

class Expr {
public:
  static bool IsField(const String& name) {
    EL_ASSERT(name.Length() > 0,
        "Cannot determine the scope of an empty name.");

    return '_' == name[0];
  }

  virtual ~Expr(void) {}

  virtual void Accept(ExprCompilerBase& compiler, int dest) const = 0;
  virtual void Trace(std::ostream& stream) const = 0;
};

std::ostream& operator<<(std::ostream& cout, const Expr& expr);

}

#endif  // __EL_EXPR_HEADER_H__
