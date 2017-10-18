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

#include <functional>
#include <system_error>
#include <Chaos/Base/UnCopyable.h>
#include "netpp_types.h"
#include "protocol.h"

namespace netpp {

class Address;
class BaseSocket;
class SocketService;

// Tcp socket acceptor
class Acceptor : private Chaos::UnCopyable {
  using AcceptHandler = std::function<void (const std::error_code&)>;

  socket_t fd_{kInvalidSocket};
  bool is_ipv6_{};
  bool non_blocking_{};
  SocketService* service_{};
public:
  using ProtocolType = Tcp;

  Acceptor(SocketService& service);
  Acceptor(SocketService& service, const ProtocolType& proto);
  Acceptor(SocketService& service, const Address& addr, bool reuse_addr = true);
  ~Acceptor(void);

  Acceptor(Acceptor&& o)
    : fd_(o.fd_)
    , is_ipv6_(o.is_ipv6_)
    , non_blocking_(o.non_blocking_)
    , service_(o.service_) {
    o.fd_ = kInvalidSocket;
    o.is_ipv6_ = 0;
    o.non_blocking_ = false;
    o.service_ = nullptr;
  }

  Acceptor& operator=(Acceptor&& o) {
    if (this != &o) {
      fd_ = o.fd_;
      is_ipv6_ = o.is_ipv6_;
      non_blocking_ = o.non_blocking_;
      service_ = o.service_;
      o.fd_ = kInvalidSocket;
      o.is_ipv6_ = 0;
      o.non_blocking_ = false;
      o.service_ = nullptr;
    }
    return *this;
  }

  void open(const ProtocolType& protocol);
  void open(const ProtocolType& protocol, std::error_code& ec);
  void close(void);
  void close(std::error_code& ec);
  void bind(const Address& addr);
  void bind(const Address& addr, std::error_code& ec);
  void listen(void);
  void listen(std::error_code& ec);
  void accept(BaseSocket& peer);
  void accept(BaseSocket& peer, std::error_code& ec);
  void async_accept(BaseSocket& peer, const AcceptHandler& handler);
  void async_accept(BaseSocket& peer, AcceptHandler&& handler);
  void accept(BaseSocket& peer, Address& addr);
  void accept(BaseSocket& peer, Address& addr, std::error_code& ec);
  void async_accept(
      BaseSocket& peer, Address& addr, const AcceptHandler& handler);
  void async_accept(BaseSocket& peer, Address& addr, AcceptHandler&& handler);
  void set_non_blocking(bool mode);
  void set_non_blocking(bool mode, std::error_code& ec);

  socket_t get_fd(void) const {
    return fd_;
  }

  bool is_open(void) const {
    return fd_ != kInvalidSocket;
  }

  bool is_non_blocking(void) const {
    return non_blocking_;
  }

  SocketService& get_service(void) {
    return *service_;
  }
};

}
