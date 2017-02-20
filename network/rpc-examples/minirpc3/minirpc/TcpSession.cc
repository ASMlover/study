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
#include "TcpSession.h"

namespace minirpc {

TcpSession::TcpSession(asio::io_service& io_service)
  : io_service_(io_service)
  , strand_(io_service)
  , socket_(io_service) {
}

TcpSession::~TcpSession(void) {
  // TODO:
}

void TcpSession::start(void) {
  // TODO:
}

void TcpSession::close(void) {
  auto self(shared_from_this());
  strand_.post([this, self] { socket_.close(); });
}

void TcpSession::write(const BufType& buf) {
  bool write_in_progress = !writbuf_queue_.empty();
  writbuf_queue_.push_back(buf);
  if (!write_in_progress)
    do_write();
}

void TcpSession::write(BufType&& buf) {
  bool write_in_progress = !writbuf_queue_.empty();
  writbuf_queue_.push_back(std::move(buf));
  if (!write_in_progress)
    do_write();
}

void TcpSession::do_write(void) {
  auto self(shared_from_this());
  asio::async_write(socket_, asio::buffer(writbuf_queue_.front()),
      [this, self](const asio::error_code& ec, std::size_t /*n*/) {
        if (!ec) {
          writbuf_queue_.pop_front();
          if (!writbuf_queue_.empty())
            do_write();
        }
        else {
          socket_.close();
        }
      });
}

}
