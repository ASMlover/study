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
#pragma once

#include "net_types.hh"

namespace coro::net {

class Socket : public Copyable {
  socket_t fd_{kInvalidSocket};
public:
  Socket() noexcept {}
  Socket(socket_t fd) noexcept : fd_(fd) {}

  inline operator bool() const noexcept { return is_valid(); }
  inline bool is_valid() const noexcept { return fd_ != kInvalidSocket; }
  inline operator socket_t() const noexcept { return get(); }
  inline socket_t get() const noexcept { return fd_; }

  bool open();
  void close();

  void set_nonblocking(bool mode = true);

  bool listen(strv_t host = "0.0.0.0", u16_t port = 5555, int backlog = 5);
  bool connect(strv_t host = "127.0.0.1", u16_t port = 5555);
  bool async_connect(strv_t host = "127.0.0.1", u16_t port = 5555);
  Socket accept();
  Socket async_accept();
  sz_t read(char* buf, sz_t len);
  sz_t async_read(char* buf, sz_t len);
  sz_t write(const char* buf, sz_t len);
  sz_t async_write(const char* buf, sz_t len);
};

}
