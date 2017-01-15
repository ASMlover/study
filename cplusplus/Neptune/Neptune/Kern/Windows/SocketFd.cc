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
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Endian.h>
#include <Neptune/Kern/SocketFd.h>

namespace Neptune {

void SocketFd::set_nonblock(int sockfd) {
  u_long val{1};
  ioctlsocket(sockfd, FIONBIO, &val);
}

void SocketFd::shutdown_read(void) {
  if (shutdown(sockfd_, SD_RECEIVE) < 0)
    CHAOSLOG_SYSERR << "SocketFd::shutdown_read - errno=" << get_errno();
}

void SocketFd::shutdown_write(void) {
  if (shutdown(sockfd_, SD_SEND) < 0)
    CHAOSLOG_SYSERR << "SocketFd::shutdown_write - errno=" << get_errno();
}

void SocketFd::shutdown_all(void) {
  if (shutdown(sockfd_, SD_BOTH) < 0)
    CHAOSLOG_SYSERR << "SocketFd::shutdown_all - errno=" << get_errno();
}

void SocketFd::close(void) {
  if (closesocket(sockfd_) < 0)
    CHAOSLOG_SYSERR << "SocketFd::close - errno=" << get_errno();
}

ssize_t SocketFd::read(std::size_t len, void* buf) {
  return recv(sockfd_, static_cast<char*>(buf), static_cast<int>(len), 0);
}

ssize_t SocketFd::write(const void* buf, std::size_t len) {
  return send(sockfd_, static_cast<const char*>(buf), static_cast<int>(len), 0);
}

void SocketFd::set_iovec(Iovec_t& vec, char* buf, size_t len) {
  vec.buf = buf;
  vec.len = len;
}

ssize_t SocketFd::readv(int niov, Iovec_t* iov) {
  DWORD read_bytes = 0;
  DWORD flags = 0;
  int rc = WSARecv(sockfd_, iov, niov, &read_bytes, &flags, NULL, NULL);
  if (rc == SOCKET_ERROR)
    return rc;
  return static_cast<ssize_t>(read_bytes);
}

bool SocketFd::set_option(int level, int optname, int optval) {
  return setsockopt(sockfd_, level, optname, (const char*)&optval, sizeof(optval)) == 0;
}

bool SocketFd::get_option(int level, int optname, int* optval, socklen_t* optlen) {
  return getsockopt(sockfd_, level, optname, (char*)optval, optlen) == 0;
}

}
