#if defined(SSER_WIN)
# include <WS2tcpip.h>
# include <WinSock2.h>

# define SHUT_RD    SD_RECEIVE
# define SHUT_WR    SD_SEND
# define SHUT_RDWR  SD_BOTH
#else
# include <arpa/inet.h>
# include <sys/select.h>
# include <netdb.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include "socket_ops.hh"

namespace sser::net::ops {

inline sockaddr_in gen_addr(strv_t host, u16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
#if defined(SSER_WIN)
  ::InetPtonA(AF_INET, host.data(), &addr.sin_addr);
#else
  ::inet_pton(AF_INET, host.data(), &addr.sin_addr);
#endif
  return addr;
}

socket_t open(int af, int type, int protocal, std::error_code& ec) {
  error::clear_errno();
  socket_t sockfd = error::wrap(::socket(af, type, protocal), ec);
  if (sockfd >= 0)
    ec = error::none();
  return sockfd;
}

int shutdown(socket_t sockfd, int how, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  error::clear_errno();
  int r = error::wrap(::shutdown(sockfd, how), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

int close(socket_t sockfd, std::error_code& ec) {
  int r{};
  if (sockfd != kINVALID) {
    error::clear_errno();
#if defined(SSER_WIN)
    r = error::wrap(::closesocket(sockfd), ec);
#else
    r = error::wrap(::close(sockfd), ec);
#endif
  }

  if (r == 0)
    ec = error::none();
  return r;
}

bool set_non_blocking(socket_t sockfd, bool non_blocking, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return false;
  }

  error::clear_errno();
#if defined(SSER_WIN)
  u_long flag = non_blocking ? 1 : 0;
  int r = error::wrap(::ioctlsocket(sockfd, FIONBIO, &flag), ec);
#else
  int r = error::wrap(::fcntl(sockfd, F_GETFL, 0), ec);
  if (r >= 0) {
    error::clear_errno();
    int flag = non_blocking ? (r | O_NONBLOCK) : (r & ~O_NONBLOCK);
    r = error::wrap(::fcntl(sockfd, F_SETFL, flag), ec);
  }
#endif
  if (r >= 0) {
    ec = error::none();
    return true;
  }
  return false;
}

int bind(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  error::clear_errno();
  auto addr = gen_addr(host, port);
  int r = error::wrap(::bind(sockfd, (const sockaddr*)&addr, sizeof(addr)), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

int listen(socket_t sockfd, int backlog, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  error::clear_errno();
  int r = error::wrap(::listen(sockfd, backlog), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

socket_t accept(socket_t sockfd, void* addr, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kINVALID;
  }

  error::clear_errno();
  socklen_t addrlen = sizeof(sockaddr_in);
  socket_t new_sockfd = error::wrap(::accept(sockfd, (sockaddr*)&addr, &addrlen), ec);
  if (new_sockfd != kINVALID)
    ec = error::none();
  return new_sockfd;
}

socket_t sync_accept(socket_t sockfd, SockState state, void* addr, std::error_code& ec) {
  for (;;) {
    socket_t new_sockfd = accept(sockfd, addr, ec);
    if (new_sockfd != kINVALID)
      return new_sockfd;

    if (ec.value() == error::WOULDBLOCK || ec.value() == error::TRYAGAIN) {
      if (state & SockState::NON_BLOCKING)
        return kINVALID;
    }
    else if (ec.value() == error::CONNECTION_ABORTED) {
      if (state & SockState::ENABLE_CONNECTION_ABORTED)
        return kINVALID;
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO) {
      if (state & SockState::ENABLE_CONNECTION_ABORTED)
        return kINVALID;
    }
#endif
    else {
      return kINVALID;
    }

    // wait for socket to become ready
    if (poll_read(sockfd, SockState::NONE, -1, ec) < 0)
      return kINVALID;
  }
}

bool non_blocking_accept(socket_t sockfd,
  SockState state, void* addr, std::error_code& ec, socket_t& new_sockfd) {
  for (;;) {
    new_sockfd = accept(sockfd, addr, ec);
    if (new_sockfd != kINVALID)
      return true;

    if (ec.value() == error::INTERRUPTED)
      continue;

    if (ec.value() == error::WOULDBLOCK || ec.value() == error::TRYAGAIN) {
    }
    else if (ec.value() == error::CONNECTION_ABORTED) {
      if (state & SockState::ENABLE_CONNECTION_ABORTED)
        return true;
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO) {
      if (state & SockState::ENABLE_CONNECTION_ABORTED)
        return true;
    }
#endif
    else {
      return true;
    }

    return false;
  }
}

int connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  error::clear_errno();
  auto addr = gen_addr(host, port);
  int r = error::wrap(::connect(sockfd, (const sockaddr*)&addr, sizeof(addr)), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

void sync_connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec) {
  connect(sockfd, host, port, ec);
  if (ec.value() != error::IN_PROGRESS && ec.value() != error::WOULDBLOCK)
    return;

  // wait for socket to become ready
  if (poll_connect(sockfd, -1, ec) < 0)
    return;

  int connerr{};
  sz_t connerr_len = sizeof(connerr);
  if (get_option(sockfd, SockState::NONE,
    SOL_SOCKET, SO_ERROR, &connerr, &connerr_len, ec) == kERROR)
    return;
  ec = error::make_err(connerr);
}

bool non_blocking_connect(socket_t sockfd, std::error_code& ec) {
  FD_SET write_fds, except_fds;
  FD_ZERO(&write_fds);
  FD_SET(sockfd, &write_fds);
  FD_ZERO(&except_fds);
  FD_SET(sockfd, &except_fds);
  timeval timeout{};
  int nready = ::select((int)(sockfd + 1), nullptr, &write_fds, &except_fds, &timeout);
  if (nready == 0)
    return false;

  // get the error code from the connect operation
  int connerr{};
  sz_t connerr_len = sizeof(connerr);
  if (get_option(sockfd,
    SockState::NONE, SOL_SOCKET, SO_ERROR, &connerr, &connerr_len, ec) == 0) {
    if (connerr)
      ec = error::make_err(connerr);
    else
      ec = error::none();
  }
  return true;
}

sz_t read(socket_t sockfd, void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return 0;
  }

  error::clear_errno();
  auto nread = error::wrap(::recv(sockfd, (char*)buf, (int)len, 0), ec);
  if (nread >= 0) {
    ec = error::none();
  }
  else {
    if (ec.value() == ERROR_NETNAME_DELETED)
      ec = error::make_err(error::CONNECTION_RESET);
    else if (ec.value() == ERROR_PORT_UNREACHABLE)
      ec = error::make_err(error::CONNECTION_REFUSED);
  }
  return nread;
}

sz_t sync_read(socket_t sockfd, SockState state, void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return 0;
  }

  if (len == 0 && (state & SockState::STREAM_ORIENTED)) {
    ec = error::none();
    return 0;
  }

  // read some data
  for (;;) {
    sz_t nread = read(sockfd, buf, len, ec);
    if (nread > 0)
      return nread;

    if ((state & SockState::STREAM_ORIENTED) && nread == 0) {
      ec = error::none();
      return 0;
    }

    if ((state & SockState::NON_BLOCKING) ||
      (ec.value() != error::WOULDBLOCK && ec.value() != error::TRYAGAIN))
      return 0;

    if (poll_read(sockfd, SockState::NONE, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_read(socket_t sockfd,
  void* buf, sz_t len, bool is_stream, std::error_code& ec, sz_t& nread) {
  for (;;) {
    sz_t bytes = read(sockfd, buf, len, ec);
    if (is_stream && bytes == 0) {
      ec = error::make_err(error::ENDOF);
      return true;
    }

    if (ec.value() == error::INTERRUPTED)
      continue;

    if (ec.value() == error::WOULDBLOCK || ec.value() == error::TRYAGAIN)
      return false;
    if (bytes >= 0) {
      ec = error::none();
      nread = bytes;
    }
    else {
      nread = 0;
    }
    return true;
  }
}

sz_t write(socket_t sockfd, const void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return 0;
  }

  error::clear_errno();
  auto nwrote = error::wrap(::send(sockfd, (const char*)buf, (int)len, 0), ec);
  if (nwrote >= 0) {
    ec = error::none();
  }
  else {
    if (ec.value() == ERROR_NETNAME_DELETED)
      ec = error::make_err(error::CONNECTION_RESET);
    else if (ec.value() == ERROR_PORT_UNREACHABLE)
      ec = error::make_err(error::CONNECTION_REFUSED);
  }
  return nwrote;
}

sz_t read_from(socket_t sockfd, void* buf, sz_t len, void* addr, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  socklen_t addrlen = sizeof(sockaddr_in);
  error::clear_errno();
  auto nread = error::wrap(::recvfrom(sockfd,
    (char*)buf, (int)len, 0, (sockaddr*)addr, &addrlen), ec);
  if (nread >= 0) {
    ec = error::none();
  }
  else {
    if (ec.value() == ERROR_NETNAME_DELETED)
      ec = error::make_err(error::CONNECTION_RESET);
    else if (ec.value() == ERROR_PORT_UNREACHABLE)
      ec = error::make_err(error::CONNECTION_REFUSED);
  }
  return nread;
}

sz_t write_to(socket_t sockfd,
  const void* buf, sz_t len, strv_t host, u16_t port, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  auto addr = gen_addr(host, port);
  error::clear_errno();
  auto nwrote = error::wrap(::sendto(sockfd,
    (const char*)buf, (int)len, 0, (const sockaddr*)&addr, sizeof(addr)), ec);
  if (nwrote >= 0) {
    ec = error::none();
  }
  else {
    if (ec.value() == ERROR_NETNAME_DELETED)
      ec = error::make_err(error::CONNECTION_RESET);
    else if (ec.value() == ERROR_PORT_UNREACHABLE)
      ec = error::make_err(error::CONNECTION_REFUSED);
  }
  return nwrote;
}

int poll_connect(socket_t sockfd, int msec, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  fd_set write_fds, except_fds;
  FD_ZERO(&write_fds);
  FD_SET(sockfd, &write_fds);
  FD_ZERO(&except_fds);
  FD_SET(sockfd, &except_fds);
  timeval timeout_obj;
  timeval* timeout{};
  if (msec >= 0) {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }

  error::clear_errno();
  int r = error::wrap(::select((int)(sockfd + 1), nullptr, &write_fds, &except_fds, timeout), ec);
  if (r >= 0)
    ec = error::none();
  return r;
}

int poll_read(socket_t sockfd, SockState state, int msec, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);
  timeval timeout_obj;
  timeval* timeout{};
  if (state & SockState::NON_BLOCKING) {
    timeout_obj.tv_sec = 0;
    timeout_obj.tv_usec = 0;
    timeout = &timeout_obj;
  }
  else if (msec >= 0) {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }

  error::clear_errno();
  int r = error::wrap(::select((int)(sockfd + 1), &fds, nullptr, nullptr, timeout), ec);
  if (r == 0)
    ec = (state & SockState::NON_BLOCKING) ? error::make_err(error::WOULDBLOCK) : error::none();
  else if (r > 0)
    ec = error::none();
  return r;
}

int set_option(socket_t sockfd, SockState& state,
  int level, int optname, const void* optval, sz_t optlen, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  if (level == kCUSTOM_SOCKOPTLEVEL && optname == kALWAYS_FAILOPT) {
    ec = error::make_err(error::INVALID_ARGUMENT);
    return kERROR;
  }
  if (level == kCUSTOM_SOCKOPTLEVEL && optname == kENABLE_CONNABORTEDOPT) {
    if (optlen != sizeof(int)) {
      ec = error::make_err(error::INVALID_ARGUMENT);
      return kERROR;
    }

    if (*as_type<const int*>(optval))
      state |= SockState::ENABLE_CONNECTION_ABORTED;
    else
      state &= ~SockState::ENABLE_CONNECTION_ABORTED;
    ec = error::none();
    return 0;
  }

  if (level == SOL_SOCKET || optname == SO_LINGER)
    state |= SockState::USER_SET_LINGER;

  error::clear_errno();
  int r = error::wrap(::setsockopt(sockfd, level, optname, (const char*)optval,(int)optlen), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

int get_option(socket_t sockfd, SockState state,
  int level, int optname, void* optval, sz_t* optlen, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  if (level == kCUSTOM_SOCKOPTLEVEL && optname == kALWAYS_FAILOPT) {
    ec = error::make_err(error::INVALID_ARGUMENT);
    return kERROR;
  }
  if (level == kCUSTOM_SOCKOPTLEVEL && optname == kENABLE_CONNABORTEDOPT) {
    if (*optlen != sizeof(int)) {
      ec = error::make_err(error::INVALID_ARGUMENT);
      return kERROR;
    }

    *as_type<int*>(optval) = (state & SockState::ENABLE_CONNECTION_ABORTED) ? 1 : 0;
    ec = error::none();
    return 0;
  }

  error::clear_errno();
  int r = error::wrap(::getsockopt(sockfd, level, optname, (char*)optval, (int*)optlen), ec);
  if (r == 0)
    ec = error::none();
  return r;
}

}