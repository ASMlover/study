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
#include <iostream>
#include "Utility.h"
#include "KcpSession.h"
#include "KcpClient.h"

namespace KcpNet {

KcpClient::KcpClient(boost::asio::io_service& io_service, std::uint16_t port)
  : socket_(io_service, udp::endpoint(udp::v4(), port))
  , timer_(io_service)
  , readbuff_(kBufferSize) {
}

KcpClient::~KcpClient(void) {
  stopped_ = true;

  socket_.close();
  timer_.cancel();
}

void KcpClient::do_write_connection(void) {
  if (stopped_ || !connecting_)
    return;

  socket_.async_send(boost::asio::buffer(make_connect_request()),
      [this](const boost::system::error_code& ec, std::size_t /*n*/) {
        if (ec)
          do_write_connection();
      });
}

void KcpClient::do_read_connection(void) {
  if (stopped_ || !connecting_)
    return;

  char buf[1024]{};
  socket_.async_receive(boost::asio::buffer(buf, sizeof(buf)),
      [this, &buf](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          std::cout << "connect response: " << buf << std::endl;

          connecting_ = false;
          connected_ = true;

          auto conv = KcpNet::get_conv_from_connect_response(buf);
          session_ = std::make_shared<KcpSession>(conv);
          session_->bind_message_functor(message_fn_);
          session_->bind_write_functor(
              [this](const KcpSessionPtr& /*s*/, const std::string& buf) {
                write(buf);
              });
          if (connection_fn_)
            connection_fn_(session_);

          do_read();
        }
        else {
          do_read_connection();
          do_write_connection();
        }
      });
}

void KcpClient::do_read(void) {
  if (stopped_ || !connected_)
    return;

  socket_.async_receive(boost::asio::buffer(readbuff_),
      [this](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          session_->input_handler(readbuff_.data(), n);
        }

        do_read();
      });
}

void KcpClient::do_timer(void) {
  if (stopped_)
    return;

  timer_.expires_from_now(boost::posix_time::milliseconds(5));
  timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
          // doing kcp update
          std::uint64_t clock = get_clock64();
          if (connecting_) {
            if (clock - connect_begtime_ > kConnectTimeout) {
              connecting_ = false;
              if (connectfail_fn_)
                connectfail_fn_();
            }
            return;
          }

          if (connected_)
            session_->update(static_cast<std::uint32_t>(clock));
        }
        do_timer();
      });
}

void KcpClient::connect(
    const std::string& remote_ip, std::uint16_t remote_port) {
  udp::endpoint remote_ep(
      boost::asio::ip::address::from_string(remote_ip), remote_port);
  socket_.async_connect(remote_ep,
      [this](const boost::system::error_code& ec) {
        if (!ec) {
          connecting_ = true;
          connect_begtime_ = get_clock64();

          do_read_connection();
          do_write_connection();
          do_timer();
        }
        else {
          if (connectfail_fn_)
            connectfail_fn_();
        }
      });
}

void KcpClient::write(const std::string& buf) {
  socket_.async_send(boost::asio::buffer(buf),
      [](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {});
}

void KcpClient::write(const char* buf, std::size_t len) {
  socket_.async_send(boost::asio::buffer(buf, len),
      [](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {});
}

}
