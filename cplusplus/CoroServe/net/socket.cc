// Copyright (c) 2020 ASMlover. All rights reserved.
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

#if defined(CORO_WIN)
# include <WS2tcpip.h>
# include <WinSock2.h>
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include "socket.hh"

namespace coro::net {

void init_addr(sockaddr_in& addr, strv_t host, u16_t port) {
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
#if defined(CORO_WIN)
  ::InetPtonA(AF_INET, host.data(), &addr.sin_addr);
#else
  ::inet_pton(AF_INET, host.data(), &addr.sin_addr);
#endif
}

bool Socket::open() {
  if (is_valid())
    return false;

  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return is_valid();
}

void Socket::close() {
  if (is_valid()) {
#if defined(CORO_WIN)
    ::shutdown(fd_, SD_BOTH);
    ::closesocket(fd_);
#else
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
#endif
    fd_ = kInvalidSocket;
  }
}

void Socket::set_nonblocking(bool mode) {
  if (!is_valid())
    return;

#if defined(CORO_WIN)
  u_long flag = mode ? 1 : 0;
  ::ioctlsocket(fd_, FIONBIO, &flag);
#else
  int flag = ::fcntl(fd_, F_GETFL, 0);
  flag = (mode ? (flag | O_NONBLOCK) : (flag & ~O_NONBLOCK));
  ::fcntl(fd_, F_SETFL, flag);
#endif
}

bool Socket::listen(strv_t host, u16_t port, int backlog) {
  if (!is_valid())
    return false;

  sockaddr_in addr;
  init_addr(addr, host, port);
  if (::bind(fd_, (const sockaddr*)&addr,
        static_cast<int>(sizeof(addr))) == kSocketError)
    return false;
  return ::listen(fd_, backlog) != kSocketError;
}

// bool Socket::connect(strv_t host = "127.0.0.1", u16_t port = 5555);
// bool Socket::async_connect(strv_t host = "127.0.0.1", u16_t port = 5555);
// Socket Socket::accept();
// Socket Socket::async_accept();
// sz_t Socket::read(char* buf, sz_t len);
// sz_t Socket::async_read(char* buf, sz_t len);
// sz_t Socket::write(const char* buf, sz_t len);
// sz_t Socket::async_write(const char* buf, sz_t len);

}
