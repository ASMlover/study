#if defined(SSER_WIN)
# include <WS2tcpip.h>
# include <WinSock2.h>

# define SHUT_RD    SD_RECEIVE
# define SHUT_WR    SD_SEND
# define SHUT_RDWR  SD_BOTH
#else
# include <arpa/inet.h>
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

socket_t accept(socket_t sockfd, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = error::make_err(error::BAD_DESCRIPTOR);
    return kINVALID;
  }

  error::clear_errno();
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  socket_t new_sockfd = error::wrap(::accept(sockfd, (sockaddr*)&addr, &addrlen), ec);
  if (new_sockfd != kINVALID)
    ec = error::none();
  return new_sockfd;
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

}