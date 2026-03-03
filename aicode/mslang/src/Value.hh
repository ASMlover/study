// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <variant>
#include <vector>
#include "Types.hh"

namespace ms {

class Object;  // forward declare

using ValueStorage = std::variant<std::monostate, bool, double, Object*>;

class Value {
  ValueStorage storage_;
public:
  // Constructors
  Value() noexcept : storage_(std::monostate{}) {}
  Value(bool b) noexcept : storage_(b) {}
  Value(double d) noexcept : storage_(d) {}
  Value(Object* obj) noexcept : storage_(obj) {}

  // Type checks
  bool is_nil() const noexcept;
  bool is_boolean() const noexcept;
  bool is_number() const noexcept;
  bool is_object() const noexcept;

  // Value extraction
  bool as_boolean() const noexcept;
  double as_number() const noexcept;
  Object* as_object() const noexcept;

  // Semantic operations
  bool is_truthy() const noexcept;
  bool is_equal(const Value& other) const noexcept;
  str_t stringify() const noexcept;

  // Convenience object type checks (forward-declared, to be implemented when Object is available)
  bool is_string() const noexcept;
  bool is_function() const noexcept;
  bool is_closure() const noexcept;
  bool is_class() const noexcept;
  bool is_instance() const noexcept;
};

// operator== for convenience
inline bool operator==(const Value& a, const Value& b) noexcept {
  return a.is_equal(b);
}

inline bool operator!=(const Value& a, const Value& b) noexcept {
  return !a.is_equal(b);
}

// ostream support
std::ostream& operator<<(std::ostream& os, const Value& value);

} // namespace ms
