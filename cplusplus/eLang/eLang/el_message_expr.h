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
#ifndef __EL_MESSAGE_EXPR_HEADER_H__
#define __EL_MESSAGE_EXPR_HEADER_H__

#include "el_expr.h"
#include "el_expr_compiler_base.h"

namespace el {

class MessageSend {
  String            name_;
  Array<Ref<Expr> > arguments_;
public:
  MessageSend(void)
    : name_()
    , arguments_() {
  }

  MessageSend(const String& name, const Array<Ref<Expr> >& arguments)
    : name_(name)
    , arguments_(arguments) {
  }

  MessageSend& operator=(const MessageSend& other) {
    if (&this != this) {
      name_ = other.name_;
      arguments_ = other.arguments_;
    }

    return *this;
  }

  inline String GetName(void) const {
    return name_;
  }

  inline Array<Ref<Expr> >& GetArguments(void) const {
    return arguments_;
  }
};

class MessageExpr : public Expr {
  Ref<Expr>          receiver_;
  Array<MessageSend> messages_;
public:
  explicit MessageExpr(Ref<Expr> receiver)
    : receiver_(receiver) {
  }

  MessageExpr(Ref<Expr> receiver,
      const String& name, const Array<Ref<Expr> >& args)
    : receiver_(receiver) {
    messages_.Append(MessageSend(name, args));
  }

  inline Ref<Expr> Receiver(void) const {
    return receiver_;
  }

  inline const Array<MessageSend>& Messages(void) const {
    return messages_;
  }

  inline void AppendSend(const String& name, const Array<Ref<Expr> >& args) {
    messages_.Append(MessageSend(name, args));
  }

  virtual void Trace(std::stream& stream) const override {
    stream << receiver_ << " ";
    for (auto i = 0; i < messages_.Count(); ++i) {
      if (i > 0)
        stream << ", ";

      const MessageSend& msg = messages_[i];
      stream << msg.GetName() << " [";
      if (msg.GetArguments().Count() > 0)
        stream << msg.GetArguments()[0];
      for (auto i = 1; i < msg.GetArguments().Count(); ++i)
        stream << "; " << msg.GetArguments()[i];
      stream << "]";
    }
  }

  EL_EXPR_VISITOR
};

}

#endif  // __EL_MESSAGE_EXPR_HEADER_H__
