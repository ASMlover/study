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
#include "buffer.h"
#include "primitive.h"
#include "address.h"
#include "socket_service.h"

namespace netpp {

bool SocketService::non_blocking(
    socket_t sockfd, bool mode, std::error_code& ec) {
  return socket::set_non_blocking(sockfd, mode, ec);
}

socket_t SocketService::open(
    int family, int socket_type, int protocol, std::error_code& ec) {
  return socket::open(family, socket_type, protocol, ec);
}

void SocketService::close(socket_t sockfd, std::error_code& ec) {
  socket::close(sockfd, ec);
}

void SocketService::shutdown(socket_t sockfd, int how, std::error_code& ec) {
  socket::shutdown(sockfd, how, ec);
}

void SocketService::bind(
    socket_t sockfd, const Address& addr, std::error_code& ec) {
  socket::bind(sockfd, addr.get_address(), ec);
}

void SocketService::listen(socket_t sockfd, std::error_code& ec) {
  socket::listen(sockfd, ec);
}

socket_t SocketService::accept(
    socket_t sockfd, Address& peer_addr, std::error_code& ec) {
  return socket::accept(sockfd, peer_addr.get_address(), ec);
}

void SocketService::async_accept(socket_t sockfd,
    BaseSocket& peer, Address& peer_addr, const AcceptHandler& handler) {
  // TODO:
}

void SocketService::async_accept(socket_t sockfd,
    BaseSocket& peer, Address& peer_addr, AcceptHandler&& handler) {
  // TODO:
}

void SocketService::connect(
    socket_t sockfd, const Address& addr, std::error_code& ec) {
  socket::sync_connect(sockfd, addr.get_address(), ec);
}

void SocketService::async_connect(
    socket_t sockfd, const Address& addr, const ConnectHandler& handler) {
  // TODO:
}

void SocketService::async_connect(
    socket_t sockfd, const Address& addr, ConnectHandler&& handler) {
  // TODO:
}

std::size_t SocketService::read(socket_t sockfd,
    const MutableBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  return socket::sync_read(sockfd,
      buf.size(), buf.data(), non_blocking, all_empty, ec);
}

std::size_t SocketService::read(socket_t sockfd,
    const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_read(sockfd, non_blocking, -1, ec);
}

void SocketService::async_read(
    socket_t sockfd, const MutableBuffer& buf, const ReadHandler& handler) {
  // TODO:
}

void SocketService::async_read(
    socket_t sockfd, const MutableBuffer& buf, ReadHandler&& handler) {
  // TODO:
}

std::size_t SocketService::write(socket_t sockfd,
    const ConstBuffer& buf, bool non_blocking, std::error_code& ec) {
  auto all_empty = buf.size() == 0;
  return socket::sync_write(sockfd,
      buf.data(), buf.size(), non_blocking, all_empty, ec);
}

std::size_t SocketService::write(socket_t sockfd,
      const NullBuffer&, bool non_blocking, std::error_code& ec) {
  return socket::poll_write(sockfd, non_blocking, -1, ec);
}

void SocketService::async_write(
    socket_t sockfd, const ConstBuffer& buf, const WriteHandler& handler) {
  // TODO:
}

void SocketService::async_write(
    socket_t sockfd, const ConstBuffer& buf, WriteHandler&& handler) {
  // TODO:
}

std::size_t SocketService::read_from(socket_t sockfd, const MutableBuffer& buf,
    Address& peer_addr, bool non_blocking, std::error_code& ec) {
  return socket::sync_read_from(sockfd,
      buf.size(), buf.data(), peer_addr.get_address(), non_blocking, ec);
}

void SocketService::async_read_from(socket_t sockfd,
    const MutableBuffer& buf, Address& peer_addr, const ReadHandler& handler) {
  // TODO:
}

void SocketService::async_read_from(socket_t sockfd,
    const MutableBuffer& buf, Address& peer_addr, ReadHandler& handler) {
  // TODO:
}

std::size_t SocketService::write_to(socket_t sockfd, const ConstBuffer& buf,
    const Address& peer_addr, bool non_blocking, std::error_code& ec) {
  return socket::sync_write(sockfd,
      buf.data(), buf.size(), peer_addr.get_address(), non_blocking, ec);
}

void SocketService::async_write_to(socket_t sockfd, const ConstBuffer& buf,
    const Address& peer_addr, const WriteHandler& handler) {
  // TODO:
}

void SocketService::async_write_to(socket_t sockfd,
    const ConstBuffer& buf, const Address& peer_addr, WriteHandler&& handler) {
  // TODO:
}

}
