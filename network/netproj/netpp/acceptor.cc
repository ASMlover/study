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
#include "netpp_internal.h"
#include "utils.h"
#include "primitive.h"
#include "address.h"
#include "socket.h"
#include "acceptor.h"

namespace netpp {

Acceptor::Acceptor(void) {
}

Acceptor::Acceptor(const ProtocolType& protocol) {
  std::error_code ec;
  fd_ = socket::open(protocol.family(),
      protocol.socket_type(), protocol.protocol(), ec);
  netpp::throw_error(ec, "open");
}

Acceptor::Acceptor(const Address& addr, bool reuse_addr) {
  std::error_code ec;
  fd_ = socket::open(addr.get_family(), SOCK_STREAM, IPPROTO_TCP, ec);
  netpp::throw_error(ec, "open");

  if (reuse_addr) {
    int optval{1};
    socket::set_option(fd_,
        SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval), ec);
    netpp::throw_error(ec, "set_option");
  }
  socket::bind(fd_, addr.get_address(), ec);
  netpp::throw_error(ec, "bind");
  socket::listen(fd_, ec);
  netpp::throw_error(ec, "listen");
}

Acceptor::~Acceptor(void) {
}

void Acceptor::open(const ProtocolType& protocol) {
  std::error_code ec;
  fd_ = socket::open(protocol.family(),
      protocol.socket_type(), protocol.protocol(), ec);
  netpp::throw_error(ec, "open");
}

void Acceptor::open(const ProtocolType& protocol, std::error_code& ec) {
  fd_ = socket::open(protocol.family(),
      protocol.socket_type(), protocol.protocol(), ec);
}

void Acceptor::close(void) {
  std::error_code ec;
  socket::close(fd_, ec);
  netpp::throw_error(ec, "close");
}

void Acceptor::close(std::error_code& ec) {
  socket::close(fd_, ec);
}

void Acceptor::bind(const Address& addr) {
  std::error_code ec;
  socket::bind(fd_, addr.get_address(), ec);
  netpp::throw_error(ec, "bind");
}

void Acceptor::bind(const Address& addr, std::error_code& ec) {
  socket::bind(fd_, addr.get_address(), ec);
}

void Acceptor::listen(void) {
  std::error_code ec;
  socket::listen(fd_, ec);
  netpp::throw_error(ec, "listen");
}

void Acceptor::listen(std::error_code& ec) {
  socket::listen(fd_, ec);
}

void Acceptor::accept(BaseSocket& peer) {
  std::error_code ec;
  auto sockfd = socket::accept(fd_, nullptr, ec);
  if (sockfd != kInvalidSocket)
    peer.attach_fd(sockfd);
  netpp::throw_error(ec, "accept");
}

void Acceptor::accept(BaseSocket& peer, std::error_code& ec) {
  auto sockfd = socket::accept(fd_, nullptr, ec);
  if (sockfd != kInvalidSocket)
    peer.attach_fd(sockfd);
}

void Acceptor::async_accept(BaseSocket& peer, const AcceptHandler& handler) {
  // TODO: need io_service
}

void Acceptor::async_accept(BaseSocket& peer, AcceptHandler&& handler) {
  // TODO: need io_service
}

void Acceptor::accept(BaseSocket& peer, Address& addr) {
  std::error_code ec;
  auto sockfd = socket::accept(fd_, addr.get_address(), ec);
  if (sockfd != kInvalidSocket)
    peer.attach_fd(sockfd);
  netpp::throw_error(ec, "accept");
}

void Acceptor::accept(BaseSocket& peer, Address& addr, std::error_code& ec) {
  auto sockfd = socket::accept(fd_, addr.get_address(), ec);
  if (sockfd != kInvalidSocket)
    peer.attach_fd(sockfd);
}

void Acceptor::async_accept(
    BaseSocket& peer, Address& addr, const AcceptHandler& handler) {
  // TODO: need io_service
}

void Acceptor::async_accept(
    BaseSocket& peer, Address& addr, AcceptHandler&& handler) {
  // TODO: need io_service
}

void Acceptor::set_non_blocking(bool mode) {
  std::error_code ec;
  if (socket::set_non_blocking(fd_, mode, ec))
    non_blocking_ = mode;
  netpp::throw_error(ec, "non_blocking");
}

void Acceptor::set_non_blocking(bool mode, std::error_code& ec) {
  if (socket::set_non_blocking(fd_, mode, ec))
    non_blocking_ = mode;
}

}
