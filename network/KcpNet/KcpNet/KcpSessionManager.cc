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
#include <ikcp.h>
#include "KcpSessionManager.h"
#include "Utility.h"

namespace KcpNet {

KcpSessionManager::KcpSessionManager(
    boost::asio::io_service& io_service,
    const std::string& address, std::uint16_t port)
  : socket_(io_service,
      udp::endpoint(boost::asio::ip::address::from_string(address), port))
  , timer_(io_service) {
  do_read();
  do_timer();
}

void KcpSessionManager::do_read(void) {
  if (stopped_)
    return;

  socket_.async_receive_from(
      boost::asio::buffer(readbuff_, sizeof(readbuff_)), sender_ep_,
      [this](const boost::system::error_code& ec, std::size_t n) {
        if (!ec && n > 0) {
          if (is_connect_packet(readbuff_, n))
            handle_connect_packet();
          else
            handle_packet(n);
        }
        else {
          // async receive error
        }

        do_read();
      });
}

void KcpSessionManager::do_timer(void) {
  if (stopped_)
    return;

  timer_.expires_from_now(boost::posix_time::milliseconds(5));
  timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec)
          container_.update_all(get_clock32());
        do_timer();
      });
}

void KcpSessionManager::handle_connect_packet(void) {
  auto conv = container_.gen_conv();
  auto buf = make_sendback_packet(conv);
  socket_.send_to(boost::asio::buffer(buf), sender_ep_);
}

void KcpSessionManager::handle_packet(std::size_t n) {
  auto conv = ikcp_getconv(readbuff_);
  auto s = container_.get(conv);
  if (!s)
    s = container_.new_session(shared_from_this(), conv, sender_ep_);

  if (s)
    s->input_handler(readbuff_, n, sender_ep_);
}

void KcpSessionManager::stop_all(void) {
  stopped_ = true;
  container_.stop_all();

  socket_.cancel();
  socket_.close();

  timer_.cancel();
}

void KcpSessionManager::call_message_functor(
    kcp_conv_t conv, SMessageType type, const std::string& buf) {
  if (message_fn_)
    message_fn_(conv, type, buf);
}

void KcpSessionManager::write_udp_buffer(
    const std::string& buf, const udp::endpoint& ep) {
  // use `async_send_to` replace `send_to`, does it work ?
  // FIXME: does it will use `sender_ep_` or `ep` ?
  // socket_.send_to(boost::asio::buffer(buf), sender_ep_);
  socket_.async_send_to(boost::asio::buffer(buf), ep,
      [](const boost::system::error_code& /*ec*/, std::size_t /*n*/) {});
}

}
