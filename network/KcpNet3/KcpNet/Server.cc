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
#include <ikcp.h>
#include "Utility.h"
#include "Session.h"
#include "Server.h"

namespace KcpNet {

Server::Server(asio::io_context& io_context, std::uint16_t port)
  : socket_(io_context, udp::endpoint(udp::v4(), port))
  , timer_(io_context)
  , readbuff_(kBufferSize) {
  do_read();
  do_timer();
}

Server::~Server(void) {
  stopped_ = true;
  sessions_.clear();

  socket_.cancel();
  socket_.close();
  timer_.cancel();
}

void Server::do_read(void) {
  if (stopped_)
    return;

  socket_.async_receive_from(asio::buffer(readbuff_), sender_ep_,
      [this](const std::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          if (is_connect_request(readbuff_.data(), n)) {
            write_udp(make_connect_response(gen_conv()), sender_ep_);
          }
          else {
            auto conv = ikcp_getconv(readbuff_.data());
            SessionPtr s{};
            auto it = sessions_.find(conv);
            if (it == sessions_.end()) {
              s = std::make_shared<Session>(conv, sender_ep_);
              s->bind_message_functor(message_fn_);
              s->bind_write_functor(
                  [this](const SessionPtr& sp, const std::string& buf) {
                    write_udp(buf, sp->get_endpoint());
                  });
              sessions_[conv] = s;

              if (connection_fn_)
                connection_fn_(s);
            }
            else {
              s = it->second;
            }
            s->input_handler(readbuff_.data(), n, sender_ep_);
          }
        }
        do_read();
      });
}

void Server::do_timer(void) {
  if (stopped_)
    return;

  timer_.expires_from_now(std::chrono::milliseconds(5));
  timer_.async_wait([this](const std::error_code& ec) {
        if (!ec) {
          for (auto& s : sessions_)
            s.second->update(get_clock32());
        }
        do_timer();
      });
}

kcp_conv_t Server::gen_conv(void) const {
  static kcp_conv_t s_conv = 1000;
  return ++s_conv;
}

void Server::write_udp(
    const char* buf, std::size_t len, const udp::endpoint& ep) {
  write_udp(std::string(buf, len), ep);
}

void Server::write_udp(const std::string& buf, const udp::endpoint& ep) {
  socket_.async_send_to(asio::buffer(buf), ep,
      [](const std::error_code& /*ec*/, std::size_t /*n*/) {});
}

}
