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

socket_t sync_accept(socket_t sockfd, bool non_blocking, void* addr, std::error_code& ec) {
  for (;;) {
    socket_t new_sockfd = accept(sockfd, addr, ec);
    if (new_sockfd != kINVALID)
      return new_sockfd;

    if (ec.value() == error::WOULDBLOCK || ec.value() == error::TRYAGAIN) {
      if (non_blocking)
        return kINVALID;
    }
    else {
      return kINVALID;
    }

    // wait for socket to become ready
    if (poll_read(sockfd, non_blocking, -1, ec) < 0)
      return kINVALID;
  }
}

bool non_blocking_accept(socket_t sockfd,
  bool non_blocking, void* addr, std::error_code& ec, socket_t& new_sockfd) {
  for (;;) {
    new_sockfd = accept(sockfd, addr, ec);
    if (new_sockfd != kINVALID)
      return true;

    if (ec.value() == error::INTERRUPTED)
      continue;

    if (ec.value() == error::WOULDBLOCK || ec.value() == error::TRYAGAIN) {
    }
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
}

bool non_blocking_connect(socket_t sockfd, std::error_code& ec) {
  // TODO:
  return false;
}

sz_t read(socket_t sockfd, void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return 0;
  }

  error::clear_errno();
  auto nread = error::wrap(::recv(sockfd, (char*)buf, (int)len, 0), ec);
  if (nread >= 0)
    ec = error::none();
  return nread;
}

sz_t write(socket_t sockfd, const void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return 0;
  }

  error::clear_errno();
  auto nwrote = error::wrap(::send(sockfd, (const char*)buf, (int)len, 0), ec);
  if (nwrote >= 0)
    ec = error::none();
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
  if (nread >= 0)
    ec = error::none();
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
  if (nwrote >= 0)
    ec = error::none();
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

int poll_read(socket_t sockfd, bool non_blocking, int msec, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);
  timeval timeout_obj;
  timeval* timeout{};
  if (non_blocking) {
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
    ec = non_blocking ? error::make_err(error::WOULDBLOCK) : error::none();
  else if (r > 0)
    ec = error::none();
  return r;
}

}