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
#include <chrono>
#include "Utility.h"
#include "Session.h"
#include "Client.h"

namespace KcpNet {

Client::Client(asio::io_context& io_context, std::uint16_t port)
  : socket_(io_context, udp::endpoint(udp::v4(), port))
  , timer_(io_context)
  , readbuff_(kBufferSize) {
}

Client::~Client(void) {
  stopped_ = false;

  socket_.cancel();
  socket_.close();
  timer_.cancel();
}

void Client::do_read_connection(void) {
  if (stopped_ || !connecting_)
    return;

  char buf[1024]{};
  socket_.async_receive(asio::buffer(buf, sizeof(buf)),
      [this, &buf](const std::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          connecting_ = false;
          connected_ = true;

          auto conv = get_conv_from_connect_response(buf);
          session_ = std::make_shared<Session>(conv);
          session_->bind_message_functor(message_fn_);
          session_->bind_write_functor(
              [this](const SessionPtr& /*s*/, const std::string& buf) {
                write_udp(buf);
              });

          if (connection_fn_)
            connection_fn_(session_);

          do_read();
        }
        else {
          do_read_connection();
          do_write_connecttion();
        }
      });
}

void Client::do_write_connecttion(void) {
  if (stopped_ || !connecting_)
    return;

  socket_.async_send(asio::buffer(make_connect_request()),
      [this](const std::error_code& ec, std::size_t /*n*/) {
        if (ec)
          do_write_connecttion();
      });
}

void Client::do_read(void) {
  if (stopped_ || !connected_)
    return;

  socket_.async_receive(asio::buffer(readbuff_),
      [this](const std::error_code& ec, std::size_t n) {
        if (!ec && n > 0)
          session_->input_handler(readbuff_.data(), n);
        do_read();
      });
}

void Client::do_timer(void) {
  if (stopped_)
    return;

  timer_.expires_from_now(std::chrono::milliseconds(5));
  timer_.async_wait([this](const std::error_code& ec) {
        if (!ec) {
          auto clock = get_clock64();
          if (connecting_) {
            if (clock - connect_begtime_ > kConnectTimeout) {
              connecting_ = false;
              if (error_fn_)
                error_fn_();
            }
            return;
          }

          if (connected_)
            session_->update(static_cast<std::uint32_t>(clock));
        }
        do_timer();
      });
}

void Client::write_udp(const char* buf, std::size_t len) {
  write_udp(std::string(buf, len));
}

void Client::write_udp(const std::string& buf) {
  socket_.async_send(asio::buffer(buf),
      [](const std::error_code& /*ec*/, std::size_t /*n*/) {});
}

void Client::connect(const std::string& remote_ip, std::uint16_t remote_port) {
  connecting_ = false;
  connected_ = false;

  udp::endpoint remote_ep(
      asio::ip::address::from_string(remote_ip), remote_port);
  socket_.async_connect(remote_ep,
      [this](const std::error_code& ec) {
        if (!ec) {
          connecting_ = true;
          connected_ = false;
          connect_begtime_ = get_clock64();

          do_read_connection();
          do_write_connecttion();
          do_timer();
        }
        else {
          if (error_fn_)
            error_fn_();
        }
      });
}

}
