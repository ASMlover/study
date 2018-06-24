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
#include "nyx_rpc_request_parser.h"

namespace nyx { namespace rpc {

rpc_request_parser::rpc_request_parser(void)
  : need_bytes_(kRpcDataLenBytes)
  , recv_limit_(kRecvLimit) {
}

std::tuple<boost::tribool, std::size_t> rpc_request_parser::parse(
    rpc_request& request, const void* data, std::size_t size) {
  boost::tribool result{false};

  char* pdata = static_cast<char*>(const_cast<void*>(data));
  switch (state_) {
  case STATE_SIZE:
    {
      auto insert_size = size;
      auto buffer_size = request.size_buf().size();

      if (need_bytes_ > size) {
        for (auto i = buffer_size - need_bytes_;
            i < buffer_size - need_bytes_ + insert_size; ++i) {
          request.size_buf()[i] = *pdata;
          ++pdata;
        }
        need_bytes_ -= insert_size;
        return std::make_tuple(boost::indeterminate, insert_size);
      }
      else {
        for (auto i = buffer_size - need_bytes_; i < buffer_size; ++i) {
          request.size_buf()[i] = *pdata;
          ++pdata;
        }

        state_ = STATE_DATA;
        need_bytes_ = request.get_size();
        if (need_bytes_ == 0 || need_bytes_ > recv_limit_) {
          return std::make_tuple(false, insert_size);
        }
        return std::make_tuple(boost::indeterminate, insert_size);
      }
    } break;
  case STATE_DATA:
    if (need_bytes_ > size) {
      auto insert_size = size;
      request.data_wbuffer().write(pdata, insert_size);
      need_bytes_ -= insert_size;
      return std::make_tuple(boost::indeterminate, insert_size);
    }
    else {
      auto insert_size = need_bytes_;
      request.data_wbuffer().write(pdata, insert_size);
      reset();

      result = true;
      return std::make_tuple(result, insert_size);
    }
    break;
  }
  return std::make_tuple(result, 0);
}

void rpc_request_parser::reset(void) {
  state_ = STATE_SIZE;
  need_bytes_ = kRpcDataLenBytes;
}

}}
