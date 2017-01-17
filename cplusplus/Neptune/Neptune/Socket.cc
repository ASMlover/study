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
#include <Chaos/Logging/Logging.h>
#include <Neptune/Kern/NetOps.h>
#include <Neptune/InetAddress.h>
#include <Neptune/Socket.h>

namespace Neptune {

Socket::~Socket(void) {
  NetOps::socket::close(sockfd_);
}

void Socket::bind_address(const InetAddress& local_addr) {
  NetOps::socket::bind(sockfd_, local_addr.get_address());
}

void Socket::listen(void) {
  NetOps::socket::listen(sockfd_);
}

int Socket::accept(InetAddress& peer_addr) {
  struct sockaddr_in6 addr6{};
  int connfd = NetOps::socket::accept(sockfd_, &addr6);
  if (connfd >= 0)
    peer_addr.set_address(addr6);
  return connfd;
}

void Socket::shutdown_write(void) {
  NetOps::socket::shutdown_write(sockfd_);
}

void Socket::set_tcp_nodelay(bool nodelay) {
  NetOps::socket::set_option(sockfd_, IPPROTO_TCP, TCP_NODELAY, nodelay ? 1 : 0);
}

void Socket::set_reuse_addr(bool reuse) {
  NetOps::socket::set_option(sockfd_, SOL_SOCKET, SO_REUSEADDR, reuse ? 1 : 0);
}

void Socket::set_reuse_port(bool reuse) {
  int r = -1;
#if defined(SO_REUSEPORT)
  r = NetOps::socket::set_option(sockfd_, SOL_SOCKET, SO_REUSEPORT, reuse ? 1 : 0);
#endif
  if (r < 0 && reuse)
    CHAOSLOG_SYSERR << "Socket::set_reuse_port - failed or not support";
}

void Socket::set_keep_alive(bool keep_alive) {
  NetOps::socket::set_option(sockfd_, SOL_SOCKET, SO_KEEPALIVE, keep_alive ? 1 : 0);
}

}
