// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Object/FunctionObject.hh>
#include <Compiler/FunctionCompiler.hh>

namespace Tadpole::Compiler {

int FunctionCompiler::resolve_local(const Lex::Token& name, const ErrorFn& errfn) {
  for (int i = locals_count() - 1; i >= 0; --i) {
    const LocalVar& local = locals_[i];
    if (local.name == name && local.depth == -1)
      errfn(Common::from_fmt("cannot load local variable `%s` in its own initializer", name.as_cstring()));
  }
  return -1;
}

int FunctionCompiler::add_upvalue(u8_t index, bool is_local) {
  for (sz_t i = 0; i < fn_->upvalues_count(); ++i) {
    if (upvalues_[i].is_equal(index, is_local))
      return Common::as_type<int>(i);
  }

  upvalues_.push_back({index, is_local});
  return Common::as_type<int>(fn_->inc_upvalues_count());
}

int FunctionCompiler::resolve_upvalue(const Lex::Token& name, const ErrorFn& errfn) {
  if (enclosing_ == nullptr)
    return -1;

  if (int local = enclosing_->resolve_local(name, errfn); local != -1) {
    enclosing_->locals_[local].is_upvalue = true;
    return add_upvalue(Common::as_type<u8_t>(local), true);
  }
  if (int upvalue = enclosing_->resolve_upvalue(name, errfn); upvalue != -1)
    return add_upvalue(Common::as_type<u8_t>(upvalue), false);

  return -1;
}

void FunctionCompiler::declare_localvar(const Lex::Token& name, const ErrorFn& errfn) {
  // TODO:
}

}
