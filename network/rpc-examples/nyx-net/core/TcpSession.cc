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
#include <iostream>
#include "SessionManager.h"
#include "TcpSession.h"

namespace nyx {

TcpSession::TcpSession(tcp::socket&& socket)
  : socket_(std::move(socket))
  , strand_(socket_.get_io_context())
  , buffer_(1024) {
}

TcpSession::~TcpSession(void) {
}

void TcpSession::do_read(void) {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
      [this, self](const std::error_code& ec, std::size_t n) {
        if (!ec) {
          handle_data(buffer_, n);
          do_read();
        }
        else {
          close();
        }
      });
}

void TcpSession::do_write(const std::string& buf) {
  auto self(shared_from_this());
  strand_.post([this, self, buf] {
      asio::async_write(socket_, asio::buffer(buf),
        [this, self](const std::error_code& /*ec*/, std::size_t /*n*/) {
        });
    });
}

void TcpSession::close(void) {
  if (closed_)
    return;
  closed_ = true;

  if (closed_fn_)
    closed_fn_(shared_from_this());

  socket_.close();
  SessionManager::get_instance().unreg_session(shared_from_this());
}

void TcpSession::handle_data(std::vector<char>& buf, std::size_t n) {
  if (message_fn_)
    message_fn_(shared_from_this(), std::string(&buf[0], n));
}

}
