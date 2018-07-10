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

#include <map>
#include <memory>
#include <variant>
#include <boost/noncopyable.hpp>
#include <boost/variant.hpp>

#include "../filter/nyx_filter.h"
#include "../filter/nyx_int_filter.h"
#include "../filter/nyx_str_filter.h"
#include "../filter/nyx_float_filter.h"

namespace nyx { namespace rpc {

class rpc_handler_userdata : private boost::noncopyable {
  using data_type = std::variant<int, float, std::string>;
  class userdata {
    data_type data_;
  public:
    userdata(int v) : data_(v) {}
    userdata(float v) : data_(v) {}
    userdata(const std::string& v) : data_(v) {}
    void set_data(int v) { data_ = v; }
    void set_data(float v) { data_ = v; }
    void set_data(const std::string& v) { data_ = v; }
    const data_type& get_data(void) const { return data_; }
  };
  using userdata_ptr = std::shared_ptr<userdata>;
  using userdata_map = std::map<std::string, userdata_ptr>;
  using userdata_pair = std::pair<std::string, userdata_ptr>;

  class rpc_handler_userdata_visitor : public boost::static_visitor<bool> {
    nyx::filter::filter_ptr filter_;
  public:
    rpc_handler_userdata_visitor(const nyx::filter::filter_ptr& f)
      : filter_(f) {
    }

    bool operator()(int v) const {
      return filter_->done(v);
    }

    bool operator()(float v) const {
      return filter_->done(v);
    }

    bool operator()(const std::string& v) const {
      return filter_->done(v);
    }
  };
  userdata_map userdata_;
public:
  template <typename T>
  void set_userdata(const std::string& k, T v) {
    auto it = userdata_.find(k);
    if (it == userdata_.end())
      userdata_.insert(userdata_pair(k, std::make_shared<userdata>(v)));
    else
      it->second->set_data(v);
  }

  bool remove_userdata(const std::string& k) {
    return userdata_.erase(k) > 0;
  }

  void clear_userdata(void) {
    userdata_.clear();
  }

  bool done(const nyx::filter::filter_ptr& filter) {
    bool r{};
    auto it = userdata_.find(filter->key());
    if (it != userdata_.end()) {
      r = std::visit([&filter](auto&& arg) -> bool {
            return filter->done(arg);
          }, it->second->get_data());
    }
    auto sub = filter->get_sub();
    if (sub) {
      if (sub->get_relation() == nyx::filter::filter::filter_relation::r_or) {
        if (!r)
          r = done(sub);
      }
      else {
        if (r)
          r = done(sub);
      }
    }
    auto next = filter->get_next();
    if (next) {
      if (next->get_relation() == nyx::filter::filter::filter_relation::r_or) {
        if (!r)
          r = done(next);
      }
      else {
        if (r)
          r = done(next);
      }
    }
    return r;
  }
};

}}
