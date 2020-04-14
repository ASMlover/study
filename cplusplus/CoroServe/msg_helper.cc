// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "msg_helper.hh"

namespace coro::msg {

std::tuple<net::Status, WriteBuf>
handle_message(net::Status status, const char* rbuf, sz_t rlen) {
  WriteBuf wbuf;
  for (sz_t i = 0; i < rlen; ++i) {
    switch (status) {
    case net::Status::INIT_ACK: break;
    case net::Status::WAIT_MSG:
      if (rbuf[i] == '^')
        status = net::Status::READ_MSG;
      break;
    case net::Status::READ_MSG:
      if (rbuf[i] == '$')
        status = net::Status::WAIT_MSG;
      else
        wbuf.push_back(rbuf[i] + 1);
      break;
    }
  }
  return std::make_tuple(status, wbuf);
}

void on_blocking_serve(coro::net::Socket& c) {
  if (auto n = c.write("*", 1); n == 0)
    return;

  net::Status status = net::Status::WAIT_MSG;
  for (;;) {
    char rbuf[1024];
    sz_t n = c.read(rbuf, sizeof(rbuf));
    if (n == 0)
      break;

    auto [st, wbuf] = handle_message(status, rbuf, n);
    status = st;
    if (!wbuf.empty())
      c.write(wbuf.data(), wbuf.size());
  }
}

}
