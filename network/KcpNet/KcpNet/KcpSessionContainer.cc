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
#include "KcpSessionContainer.h"

namespace KcpNet {

KcpSession::KcpSessionPtr KcpSessionContainer::get(kcp_conv_t conv) {
  auto it = sessions_.find(conv);
  if (it == sessions_.end())
    return KcpSession::KcpSessionPtr();
  else
    return it->second;
}

void KcpSessionContainer::update_all(std::uint32_t clock) {
  for (auto& s : sessions_)
    s.second->update(clock);
}

void KcpSessionContainer::stop_all(void) {
  // TODO:
  sessions_.clear();
}

KcpSession::KcpSessionPtr KcpSessionContainer::new_session(
    const std::weak_ptr<KcpSessionManager>& wk_connmgr,
    kcp_conv_t conv, const udp::endpoint& sender_ep) {
  auto s = KcpSession::create(wk_connmgr, conv, sender_ep);
  sessions_[conv] = s;
  return s;
}

kcp_conv_t KcpSessionContainer::gen_conv(void) const {
  static std::uint32_t s_conv = 1000;
  return ++s_conv;
}

}
