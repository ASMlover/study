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
#include <sstream>
#include "object.hh"

namespace nyx {

std::size_t Array::size(void) const {
  return sizeof(*this) + count_ * sizeof(Value);
}

std::string Array::stringify(void) const {
  // TODO:
  return "array";
}

std::size_t Forward::size(void) const {
  return sizeof(*this);
}

std::string Forward::stringify(void) const {
  std::stringstream ss;
  ss << "fwd->" << to_->address();
  return ss.str();
}

std::size_t Function::size(void) const {
  return sizeof(*this) + code_size_;
}

std::string Function::stringify(void) const {
  // TODO:
  return "function";
}

std::size_t Numeric::size(void) const {
  return sizeof(*this);
}

std::string Numeric::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

std::size_t String::size(void) const {
  return sizeof(*this) + count_;
}

std::string String::stringify(void) const {
  return chars_;
}

std::size_t TableEntries::size(void) const {
  return sizeof(*this) + count_ * sizeof(TableEntry);
}

std::string TableEntries::stringify(void) const {
  // TODO:
  return "table entries";
}

std::size_t Table::size(void) const {
  return sizeof(*this);
}

std::string Table::stringify(void) const {
  // TODO:
  return "table";
}

}
