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
#include <core/net/TcpSession.h>

namespace nyx::net {

TcpSession::TcpSession(boost::asio::io_context& context)
  : BaseSession(context)
  , socket_(context)
  , buffer_(128) {
}

TcpSession::~TcpSession(void) {
}

void TcpSession::do_async_write_impl(const std::string& buf) {
  if (nwrite_limit_ > 0 && data_queue_size_ > nwrite_limit_)
    return;

  if (is_writing_) {
    write_queue_.push_back(buf);
    data_queue_size_ += buf.size();
  }
  else {
    is_writing_ = false;
    handle_async_write(buf);
  }
}

void TcpSession::handle_async_write(const std::string& buf) {
  if (is_closed())
    return;

  auto self(shared_from_this());
  boost::asio::async_write(socket_, boost::asio::buffer(buf.data(), buf.size()),
      [this, self](const std::error_code& ec, std::size_t /*n*/) {
        if (!write_queue_.empty()) {
          auto wbuf = write_queue_.front();
          write_queue_.pop_front();
          data_queue_size_ -= wbuf.size();
          handle_async_write(wbuf);
        }
        else {
          is_writing_ = false;
          if (is_disconnected_ && invoke_shutoff())
            cleanup();
        }
      });
}

void TcpSession::cleanup(void) {
  unregister_session();
}

void TcpSession::invoke_launch(void) {
  if (is_closed())
    return;

  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](const std::error_code& ec, std::size_t n) {
        handle_async_read(ec, n);
      });
}

bool TcpSession::invoke_shutoff(void) {
  bool false_value{};
  if (!is_closed_.compare_exchange_strong(false_value,
        true, std::memory_order_release, std::memory_order_relaxed))
    return false;

  auto self(shared_from_this());
  strand_.post([this, self] {
        if (socket_.lowest_layer().is_open())
          socket_.lowest_layer().close();
      });
  return true;
}

void TcpSession::invoke_disconnect(void) {
  if (!is_disconnected_) {
    is_disconnected_ = true;
    if (!is_writing_ && invoke_shutoff())
      cleanup();
  }
}

void TcpSession::invoke_async_write(const std::string& buf) {
  do_async_write_impl(buf);
}

void TcpSession::handle_async_read(const std::error_code& ec, std::size_t n) {
  if (is_closed())
    return;

  if (!ec) {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer_),
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
