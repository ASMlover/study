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
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct ChatProtocol {
  enum ProtocolType {
    CP_UNKNOWN,
    CP_SESSION,
    CP_MESSAGE,
  };

  enum {
    NHEADER = 4,
    NBODY_MAX = 1024,
  };

  std::int16_t msglen;
  std::int16_t proto;
};

class ChatMessage {
  char data_[ChatProtocol::NHEADER + ChatProtocol::NBODY_MAX];
  int proto_{};
  std::size_t nbody_{};
public:
  char* data(void) {
    return data_;
  }

  const char* data(void) const {
    return data_;
  }

  std::size_t size(void) const {
    return ChatProtocol::NHEADER + nbody_;
  }

  char* body(void) {
    return data_ + ChatProtocol::NHEADER;
  }

  const char* body(void) const {
    return data_ + ChatProtocol::NHEADER;
  }

  void set_nbody(std::size_t new_nbody) {
    nbody_ = new_nbody;
    if (nbody_ > ChatProtocol::NBODY_MAX)
      nbody_ = ChatProtocol::NBODY_MAX;
  }

  std::size_t get_nbody(void) const {
    return nbody_;
  }

  void set_proto(int proto) {
    proto_ = proto;
  }

  int get_proto(void) const {
    return proto_;
  }

  void encode_header(void) {
    ChatProtocol cp;
    cp.proto = static_cast<std::int16_t>(proto_);
    cp.msglen = static_cast<std::int16_t>(nbody_);
    std::memcpy(data_, &cp, sizeof(cp));
  }

  bool decode_header(void) {
    ChatProtocol cp;
    std::memcpy(&cp, data_, ChatProtocol::NHEADER);
    proto_ = cp.proto;
    nbody_ = cp.msglen;
    if (nbody_ > ChatProtocol::NBODY_MAX) {
      nbody_ = 0;
      return false;
    }
    return true;
  }
};

ChatMessage gen_chat_message(int proto, const char* buf, int len) {
  ChatMessage msg;
  msg.set_proto(proto);
  msg.set_nbody(len);
  std::memcpy(msg.body(), buf, msg.get_nbody());

  return msg;
}
