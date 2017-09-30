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

namespace netpp {

class Tcp {
  int family_{};

  explicit Tcp(int family)
    : family_(family) {
  }
public:
  static Tcp v4(void);
  static Tcp v6(void);
  static Tcp get_protocol(int family);

  int socket_type(void) const;
  int protocol(void) const;

  int family(void) const {
    return family_;
  }

  friend bool operator==(const Tcp& t1, const Tcp& t2) {
    return t1.family_ == t2.family_;
  }

  friend bool operator!=(const Tcp& t1, const Tcp& t2) {
    return t1.family_ != t2.family_;
  }
};

class Udp {
  int family_{};

  explicit Udp(int family)
    : family_(family) {
  }
public:
  static Udp v4(void);
  static Udp v6(void);
  static Udp get_protocol(int family);

  int socket_type(void) const;
  int protocol(void) const;

  int family(void) const {
    return family_;
  }

  friend bool operator==(const Udp& u1, const Udp& u2) {
    return u1.family_ == u2.family_;
  }

  friend bool operator!=(const Udp& u1, const Udp& u2) {
    return u1.family_ != u2.family_;
  }
};

}
