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
#include "TcpSession.h"

namespace nyx {

TcpSession::TcpSession(tcp::socket&& socket)
  : socket_(std::move(socket))
  , buffer_(1024) {
}

TcpSession::~TcpSession(void) {
}

void TcpSession::do_read(void) {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
      [this, self](const std::error_code& ec, std::size_t /*n*/) {
        if (!ec) {
          handle_data(buffer_);
          do_read();
        }
        else {
          std::cerr << "TcpSession::do_read - read data error" << std::endl;
          socket_.close();
        }
      });
}

void TcpSession::do_write(const std::string& buf) {
  do_write(buf.data(), buf.size());
}

void TcpSession::do_write(const char* buf, std::size_t len) {
  auto self(shared_from_this());
  asio::async_write(socket_, asio::buffer(buf, len),
      [this, self](const std::error_code& /*ec*/, std::size_t /*n*/) {
      });
}

void TcpSession::handle_data(std::vector<char>& buf) {
}

}
