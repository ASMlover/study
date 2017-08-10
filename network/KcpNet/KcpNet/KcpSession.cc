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
#include "KcpSession.h"
#include "KcpSessionManager.h"

namespace KcpNet {

void KcpSession::init_kcp(kcp_conv_t conv) {
  conv_ = conv;
  kcp_ = ikcp_create(conv, this);
  kcp_->output = &KcpSession::output_handler;

  // fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
  // nodelay: 0:disable(default), 1:enable
  // interval: internal update timer interval in millisec, default is 100ms
  // resend: 0:disable fast resend(default), 1:enable fast resend
  // nc: 0:normal congestion control(default), 1:disable congestion control
  // ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc)
  ikcp_nodelay(kcp_, 1, 5, 1, 1);
}

void KcpSession::write_udp_buffer(const char* buf, std::size_t len) {
  if (auto mgr = session_mgr_.lock())
    mgr->write_udp_buffer(std::string(buf, len), sender_ep_);
}

int KcpSession::output_handler(
    const char* buf, int len, ikcpcb* /*kcp*/, void* user) {
  static_cast<KcpSession*>(user)->write_udp_buffer(buf, len);
  return 0;
}

KcpSession::KcpSession(const std::weak_ptr<KcpSessionManager>& session_mgr)
  : session_mgr_(session_mgr) {
}

KcpSession::~KcpSession(void) {
  ikcp_release(kcp_);

  kcp_ = nullptr;
  conv_ = 0;
}

KcpSession::KcpSessionPtr KcpSession::create(
    const std::weak_ptr<KcpSessionManager>& session_mgr,
    kcp_conv_t conv, const udp::endpoint& sender_ep) {
  auto s = std::make_shared<KcpSession>(session_mgr);
  if (s) {
    s->init_kcp(conv);
    s->set_sender_endpoint(sender_ep);
  }

  return s;
}

void KcpSession::update(std::uint32_t clock) {
  ikcp_update(kcp_, clock);
}

void KcpSession::input_handler(
    const char* buf, std::size_t len, const udp::endpoint& sender_ep) {
  sender_ep_ = sender_ep;
  ikcp_input(kcp_, buf, len);

  while (true) {
    char recvbuf[1024 * 100]{};
    int n = ikcp_recv(kcp_, recvbuf, sizeof(recvbuf));
    if (n > 0) {
      if (auto mgr = session_mgr_.lock()) {
        mgr->call_message_functor(conv_,
            SMessageType::MT_RECV, std::string(recvbuf, n));
      }
    }
    else {
      break;
    }
  }
}

void KcpSession::write_buffer(const std::string& buf) {
  auto nwrote = ikcp_send(kcp_, buf.data(), buf.size());
  if (nwrote < 0) {
    // TODO: need solve send error
  }
}

}
