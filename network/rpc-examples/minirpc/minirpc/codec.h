// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <cstring>
#include <vector>
#include "rpc.pb.h"

namespace minirpc {

namespace gpb = ::google::protobuf;

inline void encode(const minirpc::RpcMessage& message, std::vector<char>& buf) {
  const int nbyte = message.ByteSize();
  const int len = nbyte + 4; // RPC0
  const int ntotal = len + 4; // length prepend

  buf.resize(ntotal);
  std::memcpy(&buf[0], &ntotal, sizeof(ntotal));
  std::memcpy(&buf[4], "RPC0", 4);
  message.SerializeWithCachedSizesToArray((std::uint8_t*)&buf[8]);
}

enum ParseError {
  SUCCESS = 0,
  INVALID_LENGTH,
  INVALID_NAMELEN,
  UNKNWON_MESSAGE,
  PARSE_ERROR,
};
inline ParseError decode(const char* buf, int len, minirpc::RpcMessage& message) {
  ParseError r = SUCCESS;

  if (std::memcmp(buf, "RPC0", 4) == 0) {
    const char* data = buf + 4;
    if (message.ParseFromArray(data, len - 4))
      r = SUCCESS;
    else
      r = PARSE_ERROR;
  }
  else {
    r = UNKNWON_MESSAGE;
  }

  return r;
}

}
