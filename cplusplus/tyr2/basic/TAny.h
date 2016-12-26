// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_BASIC_ANY_HEADER_H__
#define __TYR_BASIC_ANY_HEADER_H__

#include <typeinfo>
#include <utility>
#include "TTraits.h"
#include <iostream>

namespace tyr { namespace basic {

class Any {
  class Placeholder {
  public:
    virtual ~Placeholder(void) = default;

    virtual const std::type_info& get_type(void) const = 0;
    virtual Placeholder* clone(void) const = 0;
  };

  template <typename ValueType>
  class Holder : public Placeholder {
    Holder& operator=(const Holder&) = delete;
  public:
    ValueType held_;
  public:
    Holder(const ValueType& value)
      : held_(value) {
    }

    Holder(ValueType&& value)
      : held_(std::move(value)) {
    }

    virtual const std::type_info& get_type(void) const override {
      return typeid(ValueType);
    }

    virtual Placeholder* clone(void) const override {
      return new Holder(held_);
    }
  };

  Placeholder* content_{};

  template <typename ValueType>
  friend ValueType* any_cast(Any*);

  template <typename ValueType>
  friend ValueType* unsafe_any_cast(Any*);
public:
  Any(void) = default;

  ~Any(void) {
    if (nullptr != content_)
      delete content_;
  }

  template <typename ValueType>
  Any(const ValueType& value)
    : content_(new Holder<typename std::remove_cv<typename std::decay<const ValueType>::type>::type>(value)) {
  }

  Any(const Any& r)
    : content_(r.content_ ? r.content_->clone() : nullptr) {
  }

  Any(Any&& r)
    : content_(r.content_) {
    r.content_ = nullptr;
  }

  template <typename ValueType>
  Any(ValueType&& value,
      DisableIf_t<std::is_same<Any&, ValueType>>* = nullptr,
      DisableIf_t<std::is_const<ValueType>>* = nullptr)
    : content_(new Holder<typename std::decay<ValueType>::type>(std::move(value))) {
  }

  Any& operator=(const Any& r) {
    Any(r).swap(*this);
    return *this;
  }

  Any& operator=(Any&& r) {
    r.swap(*this);
    Any().swap(r);
    return *this;
  }

  template <typename ValueType>
  Any& operator=(ValueType&& r) {
    Any(std::move(r)).swap(*this);
    return *this;
  }

  Any& swap(Any& r) {
    std::swap(content_, r.content_);
    return *this;
  }

  bool empty(void) const {
    return nullptr == content_;
  }

  void clear(void) {
    Any().swap(*this);
  }

  const std::type_info& get_type(void) const {
    return (nullptr != content_) ? content_->get_type() : typeid(void);
  }
};

class BadAnyCast : public std::bad_cast {
public:
  virtual const char* what(void) const noexcept override {
    return "tyr::basic::BadAnyCast: failed conversion using tyr::basic::any_cast";
  }
};

template <typename ValueType>
inline ValueType* any_cast(Any* operand) {
  return operand && operand->get_type() == typeid(ValueType)
    ? &static_cast<Any::Holder<typename std::remove_cv<ValueType>::type>*>(operand->content_)->held_
    : nullptr;
}

template <typename ValueType>
inline const ValueType* any_cast(const Any* operand) {
  return any_cast<ValueType>(const_cast<Any*>(operand));
}

template <typename ValueType>
ValueType any_cast(Any& operand) {
  typedef typename std::remove_reference<ValueType>::type NonRef;

  NonRef* result = any_cast<NonRef>(&operand);
  if (!result)
    throw std::exception(BadAnyCast());

  typedef typename std::conditional<
    std::is_reference<ValueType>::value,
    ValueType,
    AddReference_t<ValueType>>::type RefType;

  return static_cast<RefType>(*result);
}

template <typename ValueType>
inline ValueType any_cast(const Any& operand) {
  typedef typename std::remove_reference<ValueType>::type NonRef;
  return any_cast<const NonRef&>(const_cast<Any&>(operand));
}

template <typename ValueType>
inline ValueType any_cast(Any&& operand) {
  return any_cast<ValueType>(operand);
}

template <typename ValueType>
inline ValueType* unsafe_any_cast(Any* operand) {
  return &static_cast<Any::Holder<ValueType>*>(operand->content_)->held_;
}

template <typename ValueType>
inline const ValueType* unsafe_any_cast(const Any* operand) {
  return unsafe_any_cast<ValueType>(const_cast<Any*>(operand));
}

}}

#endif // __TYR_BASIC_ANY_HEADER_H__
