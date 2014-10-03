// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "global.h"
#include "value.h"

Value::Value(void)
  : type_(ValueType::VALUETYPE_UNDEF)
  , next_(0)
  , bool_(false) {
}

Value::Value(const Value& v)
  : type_(v.type_)
  , next_(0) {
  switch (v.type_) {
  case ValueType::VALUETYPE_BOOL:
    bool_ = v.bool_;
    break;
  case ValueType::VALUETYPE_INT:
    int_ = v.int_;
    break;
  case ValueType::VALUETYPE_REAL:
    real_ = v.real_;
    break;
  default:
    string_ = new std::string(v.string_->c_str());
  }
}

Value::Value(bool v)
  : type_(ValueType::VALUETYPE_BOOL)
  , next_(0)
  , bool_(v) {
}

Value::Value(int v)
  : type_(ValueType::VALUETYPE_INT)
  , next_(0)
  , int_(v) {
}

Value::Value(float v)
  : type_(ValueType::VALUETYPE_REAL)
  , next_(0)
  , real_(v) {
}

Value::Value(const std::string& v)
  : type_(ValueType::VALUETYPE_STR)
  , next_(0)
  , string_(new std::string(v)) {
}

Value::~Value(void) {
  if (ValueType::VALUETYPE_STR == type_ && nullptr != string_)
    delete string_;
}

Value& Value::operator=(const Value& v) {
  type_ = v.type_;

  if (ValueType::VALUETYPE_STR == type_) {
    if (nullptr != string_)
      delete string_;

    string_ = new std::string(v.string_->c_str());
  }
  else {
    switch (type_) {
    case ValueType::VALUETYPE_BOOL:
      bool_ = v.bool_;
      break;
    case ValueType::VALUETYPE_INT:
      int_ = v.int_;
      break;
    case ValueType::VALUETYPE_REAL:
      real_ = v.real_;
      break;
    default:
      break;
    }
  }

  return *this;
}

const std::string Value::ToString(void) {
  switch (type_) {
  case ValueType::VALUETYPE_BOOL:
    return bool_ ? std::string("true") : std::string("false");
  case ValueType::VALUETYPE_INT:
    return std::to_string(int_);
  case ValueType::VALUETYPE_REAL:
    return std::to_string(real_);
  case ValueType::VALUETYPE_STR:
    return std::string(string_->c_str());
  default:
    return std::string("nil");
  }
}

int Value::GetInteger(void) {
  switch (type_) {
  case ValueType::VALUETYPE_BOOL:
    return static_cast<int>(bool_);
  case ValueType::VALUETYPE_INT:
    return int_;
  case ValueType::VALUETYPE_REAL:
    return static_cast<int>(real_);
  case ValueType::VALUETYPE_STR:
    return atoi(string_->c_str());
  default:
    return 0;
  }
}

float Value::GetReal(void) {
  switch (type_) {
  case ValueType::VALUETYPE_BOOL:
    return static_cast<float>(bool_);
  case ValueType::VALUETYPE_INT:
    return static_cast<float>(int_);
  case ValueType::VALUETYPE_REAL:
    return real_;
  case ValueType::VALUETYPE_STR:
    return static_cast<float>(atof(string_->c_str()));
  default:
    return 0.0f;
  }
}

void Value::Add(Value& v) {
  if (ValueType::VALUETYPE_STR == type_) {
    string_->append(v.ToString());
  }
  else {
    if (ValueType::VALUETYPE_BOOL == type_ 
        || ValueType::VALUETYPE_INT == type_)
      int_ += v.GetInteger();
    else
      real_ += v.GetReal();
  }
}

void Value::Sub(Value& v) {
  if (ValueType::VALUETYPE_BOOL == type_
      || ValueType::VALUETYPE_INT == type_)
    int_ -= v.GetInteger();
  else if (ValueType::VALUETYPE_REAL == type_)
    real_ -= v.GetReal();
}

void Value::Mul(Value& v) {
  if (ValueType::VALUETYPE_BOOL == type_
      || ValueType::VALUETYPE_INT == type_)
    int_ *= v.GetInteger();
  else if (ValueType::VALUETYPE_REAL == type_)
    real_ *= v.GetReal();
}

void Value::Div(Value& v) {
  if (ValueType::VALUETYPE_BOOL == type_
      || ValueType::VALUETYPE_INT == type_) {
    int div = v.GetInteger();
    assert(0 != div);

    int_ /= div;
  }
  else if (ValueType::VALUETYPE_REAL == type_) {
    float div = v.GetReal();
    assert(0.0f != div);

    real_ /= div;
  }
}

void Value::Mod(Value& v) {
  if (ValueType::VALUETYPE_BOOL == type_
      || ValueType::VALUETYPE_INT == type_) {
    int mod = v.GetInteger();
    assert(0 != mod);

    int_ %= mod;
  }
}

void Value::Pow(Value& v) {
  if (type_ == v.type_
      && ValueType::VALUETYPE_BOOL != type_
      && ValueType::VALUETYPE_STR != type_) {
    if (ValueType::VALUETYPE_INT == type_)
      int_ = pow(int_, v.int_);
    else if (ValueType::VALUETYPE_REAL == type_)
      real_ = pow(real_, v.real_);
  }
}

bool Value::Is(void) const {
  switch (type_) {
  case ValueType::VALUETYPE_BOOL:
    return bool_;
  case ValueType::VALUETYPE_INT:
    return static_cast<bool>(int_);
  case ValueType::VALUETYPE_REAL:
    return static_cast<bool>(real_);
  case ValueType::VALUETYPE_STR:
    return !string_->empty();
  default:
    return false;
  }
}

bool Value::IsNot(void) const {
  return !Is();
}

bool Value::Lt(Value& v) {
  return (ValueType::VALUETYPE_INT == type_ && int_ < v.GetInteger())
    || (ValueType::VALUETYPE_REAL == type_ && real_ < v.GetReal())
    || (ValueType::VALUETYPE_STR == type_ && type_ == v.type_
        && (strcmp(string_->c_str(), v.string_->c_str()) < 0));
}

bool Value::Gt(Value& v) {
  return (ValueType::VALUETYPE_INT == type_ && int_ > v.GetInteger())
    || (ValueType::VALUETYPE_REAL == type_ && real_ > v.GetReal())
    || (ValueType::VALUETYPE_STR == type_ && type_ == v.type_
        && (strcmp(string_->c_str(), v.string_->c_str()) > 0));
}

bool Value::Le(Value& v) {
  return (ValueType::VALUETYPE_INT == type_ && int_ <= v.GetInteger())
    || (ValueType::VALUETYPE_REAL == type_ && real_ <= v.GetReal())
    || (ValueType::VALUETYPE_STR == type_ && type_ == v.type_
        && (strcmp(string_->c_str(), v.string_->c_str()) <= 0));
}

bool Value::Ge(Value& v) {
  return (ValueType::VALUETYPE_INT == type_ && int_ >= v.GetInteger())
    || (ValueType::VALUETYPE_REAL == type_ && real_ >= v.GetReal())
    || (ValueType::VALUETYPE_STR == type_ && type_ == v.type_
        && (strcmp(string_->c_str(), v.string_->c_str()) >= 0));
}

bool Value::Eq(Value& v) {
  return (type_ == v.type_)
    && ((ValueType::VALUETYPE_BOOL == type_ && bool_ == v.bool_)
        || (ValueType::VALUETYPE_INT == type_ && int_ == v.int_)
        || (ValueType::VALUETYPE_REAL == type_ && real_ == v.real_)
        || (ValueType::VALUETYPE_STR == type_ &&
          (0 == strcmp(string_->c_str(), v.string_->c_str()))));
}

bool Value::Ne(Value& v) {
  return (type_ == v.type_)
    && ((ValueType::VALUETYPE_BOOL == type_ && bool_ != v.bool_)
        || (ValueType::VALUETYPE_INT == type_ && int_ != v.int_)
        || (ValueType::VALUETYPE_REAL == type_ && real_ != v.real_)
        || (ValueType::VALUETYPE_STR == type_ &&
          (0 != strcmp(string_->c_str(), v.string_->c_str()))));
}
