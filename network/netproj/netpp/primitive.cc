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
#include "primitive.h"

namespace netpp {

namespace socket {
  socket_t open(
      int family, int socket_type, int protocol, std::error_code& ec) {
    clear_last_errno();
    auto sockfd = error_wrapper(::socket(family, socket_type, protocol), ec);
    if (sockfd == kInvalidSocket)
      return sockfd;

    ec = std::error_code();
    return sockfd;
  }

  int shutdown(socket_t sockfd, int how, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    int r = error_wrapper(::shutdown(sockfd, how), ec);
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int bind(socket_t sockfd, const void* addr, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    auto* bind_addr = static_cast<const struct sockaddr*>(addr);
    socklen_t addrlen{};
    if (bind_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    int r = error_wrapper(::bind(sockfd, bind_addr, addrlen), ec);
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int listen(socket_t sockfd, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    int r = error_wrapper(::listen(sockfd, SOMAXCONN), ec);
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  socket_t accept(
      socket_t sockfd, void* addr, std::error_code& ec, bool with_v6) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    socklen_t addrlen{};
    if (with_v6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    socket_t newfd{kInvalidSocket};
    if (addr != nullptr) {
      newfd = error_wrapper(
        ::accept(sockfd, (struct sockaddr*)addr, &addrlen), ec);
    }
    else {
      newfd = error_wrapper(::accept(sockfd, nullptr, nullptr), ec);
    }
    if (newfd == kInvalidSocket)
      return newfd;

    ec = std::error_code();
    return newfd;
  }

  socket_t sync_accept(socket_t sockfd,
      void* addr, bool non_blocking, std::error_code& ec, bool with_v6) {
    for (;;) {
      socket_t new_sockfd = socket::accept(sockfd, addr, ec, with_v6);
      if (new_sockfd != kInvalidSocket)
        return new_sockfd;

      if (ec.value() == error::TRYAGAIN || ec.value() == error::WOULD_BLOCK) {
        if (non_blocking)
          return kInvalidSocket;
      }
      else {
        return kInvalidSocket;
      }

      if (socket::poll_read(sockfd, non_blocking, -1, ec) < 0)
        return kInvalidSocket;
    }
  }

  int connect(socket_t sockfd, const void* addr, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    auto* connect_addr = static_cast<const struct sockaddr*>(addr);
    socklen_t addrlen{};
    if (connect_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    int r = error_wrapper(::connect(sockfd, connect_addr, addrlen), ec);
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int poll_connect(socket_t sockfd, int msec, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    PollFd fds(sockfd, POLLOUT);
    clear_last_errno();
    int r = error_wrapper(netpp::poll(&fds, 1, msec), ec);
    if (r >= 0)
      ec = std::error_code();
    return r;
  }

  int sync_connect(socket_t sockfd, const void* addr, std::error_code& ec) {
    socket::connect(sockfd, addr, ec);
    if (ec.value() != error::IN_PROGRESS && ec.value() != error::WOULD_BLOCK)
      return kSocketError;

    if (socket::poll_connect(sockfd, -1, ec) < 0)
      return kSocketError;

    int err{};
    std::size_t errlen = sizeof(err);
    if (socket::get_option(sockfd,
          SOL_SOCKET, SO_ERROR, &err, &errlen, ec) != 0)
      return kSocketError;
    ec = make_error(err);

    return 0;
  }

  int set_option(socket_t sockfd, int level, int optname,
      const void* optval, std::size_t optlen, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    int r = error_wrapper(::setsockopt(sockfd, level, optname,
          static_cast<const char*>(optval), static_cast<socklen_t>(optlen)), ec);
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int get_option(socket_t sockfd, int level, int optname,
      void* optval, std::size_t* optlen, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    clear_last_errno();
    auto tmp_optlen = static_cast<socklen_t>(*optlen);
    int r = error_wrapper(::getsockopt(sockfd, level, optname,
          static_cast<char*>(optval), &tmp_optlen), ec);
    *optlen = tmp_optlen;
    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int poll_read(
      socket_t sockfd, bool non_blocking, int msec, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    PollFd fds(sockfd, POLLIN);
    clear_last_errno();
    int r = error_wrapper(netpp::poll(&fds, 1, msec), ec);
    if (r == 0)
      ec = non_blocking ? netpp::make_error(EWOULDBLOCK) : std::error_code();
    else if (r > 0)
      ec = std::error_code();
    return r;
  }

  int sync_read(socket_t sockfd, std::size_t len, void* buf,
      bool non_blocking, bool all_empty, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    if (all_empty) {
      ec = std::error_code();
      return 0;
    }

    for (;;) {
      auto nread = socket::read(sockfd, len, buf, ec);
      if (nread > 0)
        return nread;

      if (non_blocking ||
          (ec.value() == error::TRYAGAIN && ec.value() == error::WOULD_BLOCK))
        return 0;

      if (socket::poll_read(sockfd, non_blocking, -1, ec) < 0)
        return 0;
    }
  }

  int poll_write(
      socket_t sockfd, bool non_blocking, int msec, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    PollFd fds(sockfd, POLLOUT);
    clear_last_errno();
    int r = error_wrapper(netpp::poll(&fds, 1, msec), ec);
    if (r == 0)
      ec = non_blocking ? netpp::make_error(EWOULDBLOCK) : std::error_code();
    else if (r > 0)
      ec = std::error_code();
    return r;
  }

  int sync_write(socket_t sockfd, const void* buf, std::size_t len,
      bool non_blocking, bool all_empty, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return kSocketError;
    }

    if (all_empty) {
      ec = std::error_code();
      return 0;
    }

    for (;;) {
      auto nwrote = socket::write(sockfd, buf, len, ec);
      if (nwrote >= 0)
        return nwrote;

      if (non_blocking ||
          (ec.value() != error::TRYAGAIN && ec.value() != error::WOULD_BLOCK))
        return 0;

      if (socket::poll_write(sockfd, non_blocking, -1, ec) < 0)
        return 0;
    }
  }

  int sync_read_from(socket_t sockfd, std::size_t len, void* buf, void* addr,
      bool non_blocking, std::error_code& ec, bool with_v6) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return 0;
    }

    for (;;) {
      auto nread = socket::read_from(sockfd, len, buf, addr, ec, with_v6);
      if (nread >= 0)
        return nread;

      if (non_blocking ||
          (ec.value() != error::TRYAGAIN && ec.value() != error::WOULD_BLOCK))
        return 0;

      if (socket::poll_read(sockfd, non_blocking, -1, ec) < 0)
        return 0;
    }
  }

  int sync_write_to(socket_t sockfd, const void* buf, std::size_t len,
      const void* addr, bool non_blocking, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = make_error(error::BAD_DESCRIPTOR);
      return 0;
    }

    for (;;) {
      auto nwrote = socket::write_to(sockfd, buf, len, addr, ec);
      if (nwrote >= 0)
        return nwrote;

      if (non_blocking ||
          (ec.value() != error::TRYAGAIN && ec.value() != error::WOULD_BLOCK))
        return 0;

      if (socket::poll_write(sockfd, non_blocking, -1, ec) < 0)
        return 0;
    }
  }
}

}
