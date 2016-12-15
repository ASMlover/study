// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include <string.h>
#include "../basic/TConfig.h"
#include "../basic/TLogging.h"
#include "TInetAddress.h"
#include "TSocketSupport.h"
#include "TSocket.h"

namespace tyr { namespace net {

Socket::~Socket(void) {
  SocketSupport::kern_close(sockfd_);
}

void Socket::bind_address(const InetAddress& localaddr) {
  SocketSupport::kern_bind(sockfd_, localaddr.get_address());
}

void Socket::listen(void) {
  SocketSupport::kern_listen(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
  struct sockaddr_in6 addr6;
  memset(&addr6, 0, sizeof(addr6));
  int connfd = SocketSupport::kern_accept(sockfd_, &addr6);
  if (connfd >= 0)
    peeraddr->set_address(addr6);
  return connfd;
}

void Socket::shutdown_write(void) {
  SocketSupport::kern_shutdown(sockfd_);
}

void Socket::set_tcp_nodelay(bool nodelay) {
  SocketSupport::kern_set_option(sockfd_, IPPROTO_TCP, TCP_NODELAY, nodelay ? 1 : 0);
}

void Socket::set_reuse_addr(bool reuse) {
  SocketSupport::kern_set_option(sockfd_, SOL_SOCKET, SO_REUSEADDR, reuse ? 1 : 0);
}

void Socket::set_reuse_port(bool reuse) {
  int ret = -1;
#ifdef SO_REUSEPORT
  ret = SocketSupport::kern_set_option(sockfd_, SOL_SOCKET, SO_REUSEPORT, reuse ? 1 : 0);
#endif
  if (ret < 0 && reuse)
    TYRLOG_SYSERR << "SO_REUSEPORT failed or not support.";
}

void Socket::set_keep_alive(bool keep_alive) {
  SocketSupport::kern_set_option(sockfd_, SOL_SOCKET, SO_KEEPALIVE, keep_alive ? 1 : 0);
}

}}
