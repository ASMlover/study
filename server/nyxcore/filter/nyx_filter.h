// Copyright (c) 2018 ASMlover. All rights reserved.
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

#include <memory>
#include <sstream>
#include <boost/noncopyable.hpp>

namespace nyx { namespace filter {

class filter;
using filter_ptr = std::shared_ptr<filter>;

class filter : private boost::noncopyable {
public:
  enum class filter_relation : int {r_and, r_or};
  enum class filter_type : int {eq = 0, ne, gt, ge, lt, le};
  using key_type = std::string;
  using key_ref = const std::string&;
protected:
  filter_type type_{filter_type::eq};
  filter_relation relation_{filter_relation::r_and};
  key_type key_;
  filter_ptr sub_;
  filter_ptr next_;
public:
  filter(filter_type t, key_ref key)
    : type_(t)
    , key_(key) {
  }

  virtual ~filter(void) {}

  key_ref key(void) const { return key_; }
  void add_sub(const filter_ptr& p) { sub_ = p; }
  void add_next(const filter_ptr& p) { next_ = p; }
  filter_ptr get_sub(void) const { return sub_; }
  filter_ptr get_next(void) const { return next_; }
  void set_relation(filter_relation r) { relation_ = r; }
  filter_relation get_relation(void) const { return relation_; }
  bool is_group(void) const { return sub_ || next_; }

  virtual bool done(int v) { return false; }
  virtual bool done(float v) { return false; }
  virtual bool done(key_ref& v) { return false; }

  virtual void print_value(std::ostringstream& oss) = 0;

  void print(bool is_root = false) {
    std::ostringstream oss;
    __print(oss, is_root);
  }
private:
  void __print(std::ostringstream& oss, bool is_root) {
    if (!is_root) {
      if (get_relation() == filter_relation::r_and)
        oss << " and ";
      else
        oss << " or ";
    }

    if (sub_)
      oss << "(";
    oss << "(" << static_cast<int>(type_) << "," << key_ << ",";
    print_value(oss);
    oss << ")";
    if (sub_) {
      sub_->__print(oss, is_root);
      oss << ")";
    }
    if (next_)
      next_->__print(oss, is_root);
  }

};

}}
