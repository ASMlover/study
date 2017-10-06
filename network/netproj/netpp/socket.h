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
class ConstBuffer;
class MutableBuffer;
class SocketService;

class BaseSocket : private Chaos::UnCopyable {
  socket_t fd_{kInvalidSocket};
  bool non_blocking_{};
  SocketService* service_{};
public:
  using TcpProtocol = Tcp;
  using UdpProtocol = Udp;

  BaseSocket(SocketService& service);
  ~BaseSocket(void);

  BaseSocket(BaseSocket&& o)
    : fd_(o.fd_)
    , non_blocking_(o.non_blocking_)
    , service_(o.service_) {
    o.fd_ = kInvalidSocket;
    o.non_blocking_ = false;
    o.service_ = nullptr;
  }

  BaseSocket& operator=(BaseSocket&& o) {
    if (this != &o) {
      fd_ = o.fd_;
      non_blocking_ = o.non_blocking_;
      service_ = o.service_;
      o.fd_ = kInvalidSocket;
      o.non_blocking_ = false;
      o.service_ = nullptr;
    }
    return *this;
  }

  void open(int family, int socket_type, int protocol);
  void open(int family, int socket_type, int protocol, std::error_code& ec);
  void close(void);
  void close(std::error_code& ec);
  void shutdown(int what);
  void shutdown(int what, std::error_code& ec);
  void bind(const Address& addr);
  void bind(const Address& addr, std::error_code& ec);
  void connect(const Address& addr);
  void connect(const Address& addr, std::error_code& ec);
  void async_connect(const Address& addr, const ConnectHandler& handler);
  void async_connect(const Address& addr, ConnectHandler&& handler);
  void non_blocking(bool mode);
  void non_blocking(bool mode, std::error_code& ec);

  socket_t get_fd(void) const {
    return fd_;
  }

  void attach_fd(socket_t fd) {
    fd_ = fd;
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

class TcpSocket : public BaseSocket {
public:
  using ProtocolType = Tcp;

  TcpSocket(SocketService& service);
  TcpSocket(SocketService& service, const ProtocolType& protocol);
  ~TcpSocket(void);

  TcpSocket(TcpSocket&& o)
    : BaseSocket(std::move(o)) {
  }

  TcpSocket& operator=(TcpSocket&& o) {
    BaseSocket::operator=(std::move(o));
    return *this;
  }

  std::size_t read(const MutableBuffer& buf);
  std::size_t read(const MutableBuffer& buf, std::error_code& ec);
  void async_read(const MutableBuffer& buf, const ReadHandler& handler);
  void async_read(const MutableBuffer& buf, ReadHandler&& handler);
  std::size_t read_some(const MutableBuffer& buf);
  std::size_t read_some(const MutableBuffer& buf, std::error_code& ec);
  std::size_t write(const ConstBuffer& buf);
  std::size_t write(const ConstBuffer& buf, std::error_code& ec);
  void async_write(const ConstBuffer& buf, const WriteHandler& handler);
  void async_write(const ConstBuffer& buf, WriteHandler&& handler);
  std::size_t write_some(const ConstBuffer& buf);
  std::size_t write_some(const ConstBuffer& buf, std::error_code& ec);
};

class UdpSocket : public BaseSocket {
public:
  using ProtocolType = Udp;

  UdpSocket(SocketService& service);
  UdpSocket(SocketService& service, const ProtocolType& protocol);
  UdpSocket(SocketService& service, const Address& addr);
  ~UdpSocket(void);

  UdpSocket(UdpSocket&& o)
    : BaseSocket(std::move(o)) {
  }

  UdpSocket& operator=(UdpSocket&& o) {
    BaseSocket::operator=(std::move(o));
    return *this;
  }

  std::size_t read(const MutableBuffer& buf);
  std::size_t read(const MutableBuffer& buf, std::error_code& ec);
  void async_read(const MutableBuffer& buf, const ReadHandler& handler);
  void async_read(const MutableBuffer& buf, ReadHandler&& handler);
  std::size_t read_from(const MutableBuffer& buf, Address& addr);
  std::size_t read_from(
      const MutableBuffer& buf, Address& addr, std::error_code& ec);
  void async_read_from(
      const MutableBuffer& buf, Address& addr, const ReadHandler& handler);
  void async_read_from(
      const MutableBuffer& buf, Address& addr, ReadHandler&& handler);
  std::size_t write(const ConstBuffer& buf);
  std::size_t write(const ConstBuffer& buf, std::error_code& ec);
  void async_write(const ConstBuffer& buf, const WriteHandler& handler);
  void async_write(const ConstBuffer& buf, WriteHandler&& handler);
  std::size_t write_to(const ConstBuffer& buf, const Address& addr);
  std::size_t write_to(
      const ConstBuffer& buf, const Address& addr, std::error_code& ec);
  void async_write_to(
      const ConstBuffer& buf, const Address& addr, const WriteHandler& handler);
  void async_write_to(
      const ConstBuffer& buf, const Address& addr, WriteHandler&& handler);
};

}
