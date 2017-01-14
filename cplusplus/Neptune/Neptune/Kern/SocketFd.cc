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
#include <Chaos/Platform.h>
#if defined(CHAOS_WINDOWS)
# include <WinSock2.h>
# include <Windows.h>
#else
# include <sys/socket.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <Chaos/Types.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Endian.h>
#include <Neptune/Kern/SocketFd.h>

namespace Neptune {

void SocketFd::open(sa_family_t family) {
  sockfd_ = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd_ < 0)
    CHAOSLOG_SYSFATAL << "SocketFd::open - create socket failed";
  set_nonblock();
}

void SocketFd::bind(const struct sockaddr* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
  if (::bind(sockfd_, addr, addrlen) < 0)
    CHAOSLOG_SYSFATAL << "SocketFd::bind - errno=" << get_errno();
}

void SocketFd::listen(void) {
  if (::listen(sockfd_, SOMAXCONN) < 0)
    CHAOSLOG_SYSFATAL << "SocketFd::listen - errno=" << get_errno();
}

int SocketFd::accept(struct sockaddr_in6* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
  int connfd = ::accept(sockfd_, cast(addr), &addrlen);
  set_nonblock(connfd);

  if (connfd < 0) {
    int saved_errno = errno;
    CHAOSLOG_SYSERR << "SocketFd::accept - accept failed, errno=" << saved_errno;
    switch (saved_errno) {
    case EAGAIN:
    case EINTR:
    case EPERM:
    case EMFILE:
    case ECONNABORTED:
    case EPROTO:
      errno = saved_errno;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOMEM:
    case ENOBUFS:
    case ENOTSOCK:
    case EOPNOTSUPP:
      CHAOSLOG_SYSFATAL << "SocketFd::accept - unexpected errno=" << saved_errno;
      break;
    default:
      CHAOSLOG_SYSFATAL << "SocketFd::accept - unknown errno=" << saved_errno;
      break;
    }
  }
  return connfd;
}

int SocketFd::connect(const struct sockaddr* addr) {
  return ::connect(sockfd_, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void SocketFd::set_nonblock(void) {
  set_nonblock(sockfd_);
}

int SocketFd::get_errno(void) {
  int optval{};
  socklen_t optlen = sizeof(optval);
  if (!get_option(SOL_SOCKET, SO_ERROR, &optval, &optlen))
    return errno;
  return optval;
}

struct sockaddr_in6 SocketFd::get_local(void) const {
  struct sockaddr_in6 addr{};
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  if (getsockname(sockfd_, cast(&addr), &addrlen) < 0)
    CHAOSLOG_SYSERR << "SocketFd::get_local";
  return addr;
}

struct sockaddr_in6 SocketFd::get_peer(void) const {
  struct sockaddr_in6 addr{};
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  if (getpeername(sockfd_, cast(&addr), &addrlen) < 0)
    CHAOSLOG_SYSERR << "SocketFd::get_peer";
  return addr;
}

bool SocketFd::is_self_connect(void) const {
  struct sockaddr_in6 local = get_local();
  struct sockaddr_in6 peer = get_peer();
  if (local.sin6_family == AF_INET) {
    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&local);
    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peer);
    return laddr4->sin_port == raddr4->sin_port
      && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  }
  else if (local.sin6_family == AF_INET6) {
    return local.sin6_port == peer.sin6_port
      && std::memcmp(&local.sin6_addr, &peer.sin6_addr, sizeof(local.sin6_addr)) == 0;
  }
  return false;
}

std::string SocketFd::to_string(const struct sockaddr* addr) {
  std::string ip = to_string(addr, true);
  const struct sockaddr_in* addr4 = to_v4(addr);
  std::uint16_t port = Neptune::n2h16(addr4->sin_port);

  char buf[64]{};
  std::snprintf(buf, sizeof(buf), "%s:%u", ip.c_str(), port);
  return buf;
}

struct sockaddr* SocketFd::cast(struct sockaddr_in* addr) {
  return static_cast<struct sockaddr*>(Chaos::implicit_cast<void*>(addr));
}

const struct sockaddr* SocketFd::cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>(Chaos::implicit_cast<const void*>(addr));
}

struct sockaddr* SocketFd::cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>(Chaos::implicit_cast<void*>(addr));
}

const struct sockaddr* SocketFd::cast(const struct sockaddr_in6* addr) {
  return static_cast<const struct sockaddr*>(Chaos::implicit_cast<const void*>(addr));
}

const struct sockaddr_in* SocketFd::to_v4(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in*>(Chaos::implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* SocketFd::to_v6(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in6*>(Chaos::implicit_cast<const void*>(addr));
}

}
