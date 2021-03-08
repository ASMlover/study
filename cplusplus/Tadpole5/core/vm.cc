// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include "chunk.hh"
#include "callframe.hh"
#include "vm.hh"

namespace tadpole {

VM::VM() noexcept {
}

VM::~VM() {
}

void VM::define_native(const str_t& name, TadpoleCFun&& fn) {}
void VM::append_object(BaseObject* o) {}
void VM::mark_object(BaseObject* o) {}
void VM::mark_value(const Value& v) {}
InterpretRet VM::interpret(const str_t& source_bytes) { return run(); }

void VM::collect() {}
void VM::reclaim_object(BaseObject* o) {}

void VM::reset() {}
void VM::runtime_error(const char* format, ...) {}

void VM::push(Value value) noexcept {}
Value VM::pop() noexcept { return stack_.back(); }
const Value& VM::peek(sz_t distance) const noexcept { return stack_[0]; }

bool VM::call(ClosureObject* closure, sz_t nargs) { return false; }
bool VM::call(const Value& callee, sz_t nargs) { return false; }
UpvalueObject* VM::capture_upvalue(Value* local) { return nullptr; }
void VM::close_upvalues(Value* last) {}

InterpretRet VM::run() { return InterpretRet::OK; }

}
