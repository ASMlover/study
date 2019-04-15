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

namespace nyx {

inline bool BaseObject::as_boolean(void) const {
  return Xptr::down<const BooleanObject>(this)->value();
}

inline double BaseObject::as_numeric(void) const {
  return Xptr::down<const NumericObject>(this)->value();
}

inline StringObject* BaseObject::as_string(void) const {
  return Xptr::down<StringObject>(const_cast<BaseObject*>(this));
}

inline const char* BaseObject::as_cstring(void) const {
  return Xptr::down<const StringObject>(this)->chars();
}

inline ClosureObject* BaseObject::as_closure(void) const {
  return Xptr::down<ClosureObject>(const_cast<BaseObject*>(this));
}

inline FunctionObject* BaseObject::as_function(void) const {
  return Xptr::down<FunctionObject>(const_cast<BaseObject*>(this));
}

inline NativeFunction BaseObject::as_native(void) const {
  return Xptr::down<const NativeObject>(this)->get_function();
}

inline ClassObject* BaseObject::as_class(void) const {
  return Xptr::down<ClassObject>(const_cast<BaseObject*>(this));
}

inline InstanceObject* BaseObject::as_instance(void) const {
  return Xptr::down<InstanceObject>(const_cast<BaseObject*>(this));
}

inline BoundMethodObject* BaseObject::as_bound_method(void) const {
  return Xptr::down<BoundMethodObject>(const_cast<BaseObject*>(this));
}

}
