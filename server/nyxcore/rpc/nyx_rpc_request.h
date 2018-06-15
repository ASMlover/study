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

#include <array>
#include <sstream>
#include <boost/noncopyable.hpp>
#include "nyx_rpc_common.h"

namespace nyx { namespace rpc {

class rpc_request : private boost::noncopyable {
  using size_buffer = std::array<char, kRpcDataLenBytes>;

  std::stringstream data_;
  size_buffer size_;

  void reset_size(void) {
    for (auto i = 0u; i < size_.size(); ++i)
      size_[i] = 0;
  }
public:
  rpc_request(void)
    : data_(std::ios_base::out | std::ios_base::in | std::ios_base::binary) {
    reset_size();
  }

  size_buffer& size_buff(void) {
    return size_;
  }

  std::istream& data_rbuffer(void) {
    return data_;
  }

  std::ostream& data_wbuffer(void) {
    return data_;
  }

  std::stringstream& buffer(void) {
    return data_;
  }

  void reset(void) {
    reset_size();
    data_.str("");
    data_.clear();
  }

  std::uint32_t get_size(void);
};

}}
