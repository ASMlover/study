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
#ifndef __EL_ARRAY_OBJECT_HEADER_H__
#define __EL_ARRAY_OBJECT_HEADER_H__

namespace el {

class ArrayObject : public Object {
  Array<Value> elements_;
public:
  ArrayObject(const Value& parent, int length)
    : Object(parent)
    , elements_(length) {
  }

  inline Array<Value>& Elements(void) const {
    return elements_;
  }

  virtual String AsString(void) const override {
    String text = "#[";

    if (elements_.Count() > 0)
      text += elements_[0].AsString();
    for (auto i = 1; i < elements_.Count(); ++i)
      text += String(", ") + elements_[i].AsString();
    text += "]";

    return text;
  }

  virtual ArrayObject* AsArray(void) override {
    return this;
  }

  virtual void Trace(std::ostream& stream) const override {
    stream << AsString();
  }
};

}

#endif  // __EL_ARRAY_OBJECT_HEADER_H__
