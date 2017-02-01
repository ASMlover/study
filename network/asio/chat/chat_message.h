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
#ifndef CHATMESSAGE_H_
#define CHATMESSAGE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>

class ChatMessage {
  enum {
    NHEADER = 4,
    MAX_NBODY = 512,
  };
  char data_[NHEADER + MAX_NBODY]{};
  std::size_t nbody_{};
public:
  ChatMessage(void) {
  }

  char* data(void) {
    return data_;
  }

  const char* data(void) const {
    return data_;
  }

  std::size_t size(void) const {
    return NHEADER + nbody_;
  }

  char* body(void) {
    return data_ + NHEADER;
  }

  const char* body(void) const {
    return data_ + NHEADER;
  }

  std::size_t get_nheader(void) const {
    return NHEADER;
  }

  std::size_t get_nbody(void) const {
    return nbody_;
  }

  void set_nbody(std::size_t new_len) {
    nbody_ = new_len;
    if (nbody_ > MAX_NBODY)
      nbody_ = MAX_NBODY;
  }

  void encode_header(void) {
    char header[NHEADER + 1]{};
    std::snprintf(header, sizeof(header), "%4d", static_cast<int>(nbody_));
    std::memcpy(data_, header, NHEADER);
  }

  bool decode_header(void) {
    char header[NHEADER + 1]{};
    std::strncat(header, data_, NHEADER);
    nbody_ = std::atoi(header);
    if (nbody_ > MAX_NBODY) {
      nbody_ = 0;
      return false;
    }
    return true;
  }
};

#endif // CHATMESSAGE_H_
