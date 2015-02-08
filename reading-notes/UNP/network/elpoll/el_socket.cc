// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "el_poll.h"
#include "el_address.h"
#include "el_socket.h"

namespace el {

Socket::Socket(void)
  : fd_(EL_NETINVAL) {
}

Socket::~Socket(void) {
  Close();
}

bool Socket::SetTcpNoDelay(bool nodelay) {
  return SetOption(IPPROTO_TCP, TCP_NODELAY, (nodelay ? 1 : 0));
}

bool Socket::SetReuseAddr(bool reuse) {
  return SetOption(SOL_SOCKET, SO_REUSEADDR, (reuse ? 1 : 0));
}

bool Socket::SetKeepAlive(bool alive) {
  return SetOption(SOL_SOCKET, SO_KEEPALIVE, (alive ? 1 : 0));
}

bool Socket::Open(void) {
  return (EL_NETINVAL !=
      (fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)));
}

bool Socket::Bind(const char* addr, uint16_t port) {
  if (EL_NETINVAL == fd_)
    return false;

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr =
    (nullptr == addr ? htonl(INADDR_ANY) : inet_addr(addr));
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  return (EL_NETERR != bind(fd_,
        (struct sockaddr*)&host_addr, sizeof(host_addr)));
}

bool Socket::Listen(void) {
  if (EL_NETINVAL == fd_)
    return false;

  return (EL_NETERR != listen(fd_, SOMAXCONN));
}

bool Socket::Accept(Socket& connector, Address& addr) {
  if (EL_NETINVAL == fd_)
    return false;

  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);
  int fd = accept(fd_, (struct sockaddr*)&remote_addr, &addrlen);
  if (EL_NETINVAL == fd)
    return false;

  connector.Attach(fd);
  addr.Attach(remote_addr);

  return true;
}

bool Socket::Connect(const char* addr, uint16_t port) {
  if (EL_NETINVAL == fd_)
    return false;

  if (nullptr == addr)
    addr = "127.0.0.1";
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(addr);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);
  return (EL_NETERR != connect(fd_,
        (struct sockaddr*)&remote_addr, sizeof(remote_addr)));
}

int Socket::Get(int bytes, char* buffer) {
  if (EL_NETINVAL == fd_ || bytes <= 0 || nullptr == buffer)
    return EL_NETERR;

  return recv(fd_, buffer, bytes, 0);
}

int Socket::Put(const char* buffer, int bytes) {
  if (EL_NETINVAL == fd_ || nullptr == buffer || bytes <= 0)
    return EL_NETERR;

  return send(fd_, buffer, bytes, 0);
}

}
