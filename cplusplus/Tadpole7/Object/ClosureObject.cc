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
#include <GC/GC.hh>
#include <Object/ClosureObject.hh>

namespace Tadpole::Object {

ClosureObject::ClosureObject(FunctionObject* fn) noexcept
  : BaseObject(ObjType::CLOSURE), fn_(fn) {
  if (sz_t n = fn->upvalues_count(); n > 0) {
    upvalues_ = new UpvalueObject*[n];
    for (sz_t i = 0; i < n; ++i)
      upvalues_[i] = nullptr;
  }
}

ClosureObject::~ClosureObject() {
  if (upvalues_ != nullptr)
    delete [] upvalues_;
}

str_t ClosureObject::stringify() const {
  ss_t ss;
  ss << "<closure function `" << fn_->name_asstr() << "` at `" << this << "`>";
  return ss.str();
}

void ClosureObject::iter_children(ObjectVisitor&& visitor) {
  visitor(fn_);
  for (sz_t i = 0; i < upvalues_count(); ++i)
    visitor(upvalues_[i]);
}

ClosureObject* ClosureObject::create(FunctionObject* fn) {
  return GC::make_object<ClosureObject>(fn);
}

}
