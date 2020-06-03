#if defined(SEL_WIN)
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
#include "net_error.hh"
#include "net_socket.hh"

namespace sel::net {

inline void clear_errno() noexcept {
#if defined(SEL_WIN)
  ::WSASetLastError(0);
#else
  errno = 0;
#endif
}

inline int get_errno() noexcept {
#if defined(SEL_WIN)
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

inline std::error_code make_error(int ec) noexcept {
  return std::error_code(ec, std::system_category());
}

template <typename Ret>
inline Ret error_wrap(Ret r, std::error_code& ec) noexcept {
  ec = make_error(get_errno());
  return r;
}

inline void throw_error(const std::error_code& ec) {
  if (ec)
    throw std::system_error(ec);
}

inline void throw_error(const std::error_code& ec, const char* location) {
  if (ec)
    throw std::system_error(ec, string_format("%s: %s", location, ec.message().c_str()));
}

namespace impl {

inline void set_addr(sockaddr_in& addr, strv_t host, u16_t port) {
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
#if defined(SEL_WIN)
  ::InetPtonA(AF_INET, host.data(), &addr.sin_addr);
#else
  ::inet_pton(AF_INET, host.data(), &addr.sin_addr);
#endif
}

socket_t open(int af, int type, int protocol, std::error_code& ec) {
  clear_errno();
  socket_t sockfd = error_wrap(::socket(af, type, protocol), ec);
  if (sockfd != kINVALID)
    ec = std::error_code();

  return sockfd;
}

int shutdown(socket_t sockfd, int how, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  clear_errno();
  int r = error_wrap(::shutdown(sockfd, how), ec);
  if (r == 0)
    ec = std::error_code();

  return r;
}

int close(socket_t sockfd, std::error_code& ec) {
  int r{};
  if (sockfd != kINVALID) {
    clear_errno();
#if defined(SEL_WIN)
    r = error_wrap(::closesocket(sockfd), ec);
#else
    r = error_wrap(::close(sockfd), fd);
#endif

    if (r == 0)
      ec = std::error_code();
  }
  return r;
}

bool setblocking(socket_t sockfd, bool mode, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return false;
  }

  clear_errno();
#if defined(SEL_WIN)
  u_long flag = mode ? 0 : 1;
  int r = error_wrap(::ioctlsocket(sockfd, FIONBIO, &flag), ec);
#else
  int r = error_wrap(::fcntl(sockfd, F_GETFL, 0), ec);
  if (r >= 0) {
    clear_errno();
    int flag = mode ? (r & ~O_NONBLOCK) : (r | O_NONBLOCK);
    r = error_wrap(::fcntl(sockfd, F_SETGL, flag), ec);
  }
#endif
  if (r >= 0) {
    ec = std::error_code();
    return true;
  }
  return false;
}

int bind(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  sockaddr_in bind_addr;
  set_addr(bind_addr, host, port);

  clear_errno();
  int r = error_wrap(::bind(sockfd, (const sockaddr*)&bind_addr, sizeof(bind_addr)), ec);
  if (r == 0)
    ec = std::error_code();
  return r;
}

int listen(socket_t sockfd, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  clear_errno();
  int r = error_wrap(::listen(sockfd, SOMAXCONN), ec);
  if (r == 0)
    ec = std::error_code();
  return r;
}

socket_t accept(socket_t sockfd, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kINVALID;
  }

  sockaddr_in peer_addr;
  socklen_t addr_len = sizeof(peer_addr);

  clear_errno();
  socket_t newfd = error_wrap(::accept(sockfd, (sockaddr*)&peer_addr, &addr_len), ec);
  if (newfd != kINVALID)
    ec = std::error_code();
  return newfd;
}

int connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  sockaddr_in host_addr;
  set_addr(host_addr, host, port);
  socklen_t addr_len = sizeof(host_addr);

  clear_errno();
  int r = error_wrap(::connect(sockfd, (const sockaddr*)&host_addr, addr_len), ec);
  if (r == 0)
    ec = std::error_code();
  return r;
}

int read(socket_t sockfd, sz_t len, void* buf, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  clear_errno();
  auto nread = error_wrap(::recv(sockfd, as_type<char*>(buf), as_type<int>(len), 0), ec);
  if (nread >= 0)
    ec = std::error_code();
  return nread;
}

int write(socket_t sockfd, const void* buf, sz_t len, std::error_code& ec) {
  if (sockfd == kINVALID) {
    ec = make_error(error::BAD_DESCRIPTOR);
    return kERROR;
  }

  clear_errno();
  auto nwrote = error_wrap(::send(sockfd, as_type<const char*>(buf), as_type<int>(len), 0), ec);
  if (nwrote >= 0)
    ec = std::error_code();
  return nwrote;
}

}

void Socket::open() {
  if (is_valid())
    return;

  std::error_code ec;
  sockfd_ = impl::open(AF_INET, SOCK_STREAM, IPPROTO_TCP, ec);
  throw_error(ec, "open");
}

void Socket::open(std::error_code& ec) {
  sockfd_ = impl::open(AF_INET, SOCK_STREAM, IPPROTO_TCP, ec);
}

void Socket::close() {
  std::error_code ec;
  impl::shutdown(sockfd_, SHUT_RDWR, ec);
  throw_error(ec, "close(shutdown)");

  impl::close(sockfd_, ec);
  throw_error(ec, "close(close)");
}

void Socket::close(std::error_code& ec) {
  int r = impl::shutdown(sockfd_, SHUT_RDWR, ec);
  if (r == 0)
    impl::close(sockfd_, ec);
}

void Socket::setblocking(bool is_blocking) {
  std::error_code ec;
  if (impl::setblocking(sockfd_, is_blocking, ec))
    blocking_ = is_blocking;
  throw_error(ec, "setblocking");
}

void Socket::setblocking(bool is_blocking, std::error_code& ec) {
  if (impl::setblocking(sockfd_, is_blocking, ec))
    blocking_ = is_blocking;
}

void Socket::listen(strv_t host, u16_t port) {
  std::error_code ec;
  impl::bind(sockfd_, host, port, ec);
  throw_error(ec, "listen(bind)");

  auto r = impl::listen(sockfd_, ec);
  throw_error(ec, "listen(listen)");
}

void Socket::listen(strv_t host, u16_t port, std::error_code& ec) {
  int r = impl::bind(sockfd_, host, port, ec);
  if (r == 0)
    impl::listen(sockfd_, ec);
}

void Socket::connect(strv_t host, u16_t port) {
  std::error_code ec;
  impl::connect(sockfd_, host, port, ec);
  throw_error(ec, "connect");
}

void Socket::connect(strv_t host, u16_t port, std::error_code& ec) {
  impl::connect(sockfd_, host, port, ec);
}

socket_t Socket::accept() {
  std::error_code ec;
  socket_t sockfd = impl::accept(sockfd_, ec);
  throw_error(ec, "accept");
  return sockfd;
}

socket_t Socket::accept(std::error_code& ec) {
  socket_t sockfd = impl::accept(sockfd_, ec);
  return sockfd;
}

sz_t Socket::read(char* buf, sz_t len) {
  std::error_code ec;
  auto nread = impl::read(sockfd_, len, buf, ec);
  throw_error(ec, "read");
  return nread;
}

sz_t Socket::read(char* buf, sz_t len, std::error_code& ec) {
  return impl::read(sockfd_, len, buf, ec);
}

sz_t Socket::write(const char* buf, sz_t len) {
  std::error_code ec;
  auto nwrote = impl::write(sockfd_, buf, len, ec);
  throw_error(ec, "write");
  return nwrote;
}

sz_t Socket::write(const char* buf, sz_t len, std::error_code& ec) {
  return impl::write(sockfd_, buf, len, ec);
}

}