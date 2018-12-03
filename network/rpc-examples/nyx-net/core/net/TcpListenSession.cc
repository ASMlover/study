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
#include <core/net/TcpListenSession.h>

namespace nyx::net {

TcpListenSession::TcpListenSession(asio::io_context& context)
  : TcpSession(context) {
}

TcpListenSession::~TcpListenSession(void) {
}

void TcpListenSession::set_callback_handler(const HandlerPtr& handler) {
  handler_ = handler;
}

void TcpListenSession::notify_new_connection(void) {
  if (handler_ && handler_->on_new_connection)
    handler_->on_new_connection(shared_from_this());
}

bool TcpListenSession::invoke_shutoff(void) {
  if (handler_ && handler_->on_disconnected)
    handler_->on_disconnected(shared_from_this());
  return TcpSession::invoke_shutoff();
}

void TcpListenSession::handle_async_read(
    const std::error_code& ec, std::size_t n) {
  if (is_closed())
    return;

  if (!ec) {
    if (handler_ && handler_->on_message)
      handler_->on_message(shared_from_this(), std::string(buffer_.data(), n));

    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(buffer_),
        [this, self](const std::error_code& ec, std::size_t n) {
          handle_async_read(ec, n);
        });
  }
  else {
    if (invoke_shutoff())
      cleanup();
  }
}

}
