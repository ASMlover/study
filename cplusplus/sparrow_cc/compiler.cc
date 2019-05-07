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
#include <memory>
#include "lexer.hh"
#include "module_object.hh"
#include "value.hh"
#include "compiler.hh"

namespace sparrow {

struct CompileUnit {
  FunctionObject* fn;
  std::vector<LocalVar> locals;
  std::vector<Upvalue> upvalues;
  int scope_depth{};
  int slot_count{};
  Loop* curr_loop{};

  ClassInfo* enclosing_class{};
  CompileUnit* encloding_unit{};

  // std::shared_ptr<Lexer>& curr_lex;
  Lexer* curr_lex{};
};

int Compiler::def_modvar(VM& vm,
    ModuleObject* mod, const str_t& name, const Value& value) {
  int index = mod->index_of(name);
  if (index == -1)
    index = mod->append_var(name, value);
  else if (mod->get_value(index).is_numeric())
    mod->assign_var(index, value);
  else
    index = -1;

  return index;
}

}
