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
#include "TcpSession.h"

namespace nyx {

TcpSession::TcpSession(asio::io_context& context)
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
}

void TcpSession::start_impl(void) {
  auto self(shared_from_this());
  socket_.async_write_some(asio::buffer(buffer_),
      [this, self](const std::error_code& ec, std::size_t /*n*/) {
        if (!ec)
          start_impl();
      });
}

bool TcpSession::stop_impl(void) {
  bool false_value{};
  if (!is_closed_.compare_exchange_strong(false_value,
        true, std::memory_order_release, std::memory_order_relaxed))
    return false;

  auto self(shared_from_this());
  strand_.post([this, self] { socket_.close(); });
  return true;
}

void TcpSession::disconnect_impl(void) {
  if (!is_disconnected_) {
    is_disconnected_ = true;
    if (!is_writing_ && stop_impl())
      unregister_session();
  }
}

void TcpSession::async_write_impl(const std::string& buf) {
}

}
