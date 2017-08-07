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
#include "KcpClient.h"
#include "Utility.h"

namespace KcpNet {

static constexpr std::uint64_t kConnectTimeout = 5000; // milliseconds
static constexpr std::uint64_t kConnectRequestInterval = 500; // milliseconds

int KcpClient::output_callback(
    const char* buf, int len, ikcpcb* /*kcp*/, void* user) {
  static_cast<KcpClient*>(user)->write_package(buf, len);
  return 0;
}

void KcpClient::init_kcp(kcp_conv_t conv) {
  kcp_ = ikcp_create(conv, (void*)this);
  kcp_->output = &KcpClient::output_callback;

  // fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
  // nodelay: 0:disable(default), 1:enable
  // interval: internal update timer interval in millisec, default is 100ms
  // resend: 0:disable fast resend(default), 1:enable fast resend
  // nc: 0:normal congestion control(default), 1:disable congestion control
  // ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc)
  ikcp_nodelay(kcp_, 1, 5, 1, 1);
}

void KcpClient::do_connect(std::uint64_t current_clock) {
  if (current_clock - connect_begtime_ > kConnectTimeout) {
    connecting_ = false;
    message_fn_(0, CMessageType::MT_CONNECTFAIL, KCPNET_CONNECT_TIMEOUT, this);
    return;
  }

  if (current_clock - connect_last_reqtime_ > kConnectRequestInterval)
    do_connect_resend_packet(current_clock);
  try_recv_connect_back_packet();
}

void KcpClient::do_connect_resend_packet(std::uint64_t current_clock) {
  connect_last_reqtime_ = current_clock;

  std::string msg(make_connect_packet());
  socket_.send(boost::asio::buffer(msg));
}

void KcpClient::try_recv_connect_back_packet(void) {
  char recv_buf[1024]{};
  socket_.receive(boost::asio::buffer(recv_buf, sizeof(recv_buf)));

  auto len = std::strlen(recv_buf);
  if (len > 0 && is_connect_sendback_packet(recv_buf, len)) {
    auto conv = get_conv_from_sendback_packet(recv_buf);
    init_kcp(conv);
    connecting_ = false;
    connected_ = true;

    do_async_receive();
  }
}

void KcpClient::do_async_receive(void) {
  if (stopped_)
    return;

  socket_.async_receive_from(
      boost::asio::buffer(data_, sizeof(data_)), remote_ep_,
      [this](boost::system::error_code ec, std::size_t n) {
        if (!ec && n > 0) {
          ikcp_input(kcp_, data_, n);

          while (true) {
            std::string msg(recv_package_from_kcp());
            if (msg.size() > 0)
              message_fn_(kcp_->conv, CMessageType::MT_RECV, msg, this);
            else
              break;
          }
        }
        do_async_receive();
      });
}

std::string KcpClient::recv_package_from_kcp(void) {
  char buf[1024 * 10]{};
  auto len = ikcp_recv(kcp_, buf, sizeof(buf));
  if (len < 0)
    return "";

  return std::string(buf, len);
}

void KcpClient::write_package(const char* buf, int len) {
  socket_.send(boost::asio::buffer(buf, len));
}

KcpClient::KcpClient(
    boost::asio::io_service& io_service, std::uint16_t bind_port)
  : socket_(io_service, udp::endpoint(udp::v4(), bind_port)) {
  do_async_receive();
}

void KcpClient::stop(void) {
  socket_.cancel();
  socket_.close();
  stopped_ = true;
}

void KcpClient::connect_async(
    const std::string& remote_ip, std::uint16_t remote_port) {
  remote_ep_ = udp::endpoint(
      boost::asio::ip::address::from_string(remote_ip), remote_port);
  socket_.connect(remote_ep_);

  connecting_ = true;
  connect_begtime_ = get_clock64();
}

void KcpClient::update(void) {
  uint64_t current_clock = get_clock64();
  if (connecting_) {
    do_connect(current_clock);
    return;
  }

  if (connected_)
    ikcp_update(kcp_, current_clock);
}

void KcpClient::write_buffer(const std::string& buf) {
  ikcp_send(kcp_, buf.data(), buf.size());
}

}
