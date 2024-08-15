// Copyright (c) 2024 ASMlover. All rights reserved.
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

#include <algorithm>
#include <typeinfo>
#include "common.hh"

namespace ev {

class Any : public Copyable {
protected:
  struct PlaceHolder {
    virtual ~PlaceHolder() {}
    virtual const std::type_info& get_type() const = 0;
    virtual PlaceHolder* clone() const = 0;
  };

  template <typename ValueType> class Holder final : public PlaceHolder {
  public:
    ValueType held_;

    Holder(const ValueType& value) : held_{value} {}

    virtual const std::type_info& get_type() const override {
      return typeid(ValueType);
    }

    virtual PlaceHolder* clone() const override {
      return new Holder(held_);
    }
  };
protected:
  PlaceHolder* content_{};

  template <typename ValueType>
  friend ValueType* any_cast(Any*) noexcept;
public:
  Any() noexcept {}

  ~Any() noexcept {
    if (nullptr != content_)
      delete content_;
  }

  template <typename ValueType>
  Any(const ValueType& value) noexcept : content_{new Holder<ValueType>(value)} {
  }

  inline Any& swap(Any& r) noexcept {
    std::swap(content_, r.content_);
    return *this;
  }

  inline Any& operator=(const Any& r) noexcept {
    Any(r).swap(*this);
    return *this;
  }

  template <typename ValueType> inline Any& operator=(const ValueType& r) noexcept {
    Any(r).swap(*this);
    return *this;
  }

  inline bool is_empty() const noexcept {
    return !content_;
  }

  inline const std::type_info& get_type() const noexcept {
    return content_ ? content_->get_type() : typeid(void);
  }

  template <typename ValueType> inline ValueType get() const noexcept {
    if (get_type() == typeid(ValueType))
      return static_cast<Any::Holder<ValueType>*>(content_)->held_;
    else
      return ValueType();
  }

  template <typename ValueType> inline ValueType operator()() const noexcept {
    return get<ValueType>();
  }
};

template <typename ValueType> inline ValueType* any_cast(Any* any) noexcept {
  if (any && any->get_type() == typeid(ValueType))
    return &(static_cast<Any::Holder<ValueType>*>(any->content_)->held_);
  return nullptr;
}

template <typename ValueType> inline const ValueType* any_cast(const Any* any) noexcept {
  return any_cast<ValueType>(const_cast<Any*>(any));
}

template <typename ValueType> ValueType any_cast(const Any& any) noexcept {
  const ValueType* result = any_cast<ValueType>(&any);
  assert(result);

  if (!result)
    return ValueType();
  return *result;
}

}
