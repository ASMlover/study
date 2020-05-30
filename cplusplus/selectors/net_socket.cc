#if defined(SEL_WIN)
# include <WS2tcpip.h>
# include <WinSock2.h>

# define NATIVE_ERR(e)                e
# define SOCKET_ERR(e)                WSA##e
# define WIN_OR_POSIX(e_win, e_posix) e_win

# define SHUT_RDWR                    SD_BOTH
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <fcntl.h>
# include <unistd.h>

# define NATIVE_ERR(e)                e
# define SOCKET_ERR(e)                e
# define WIN_OR_POSIX(e_win, e_posix) e_posix
#endif
#include <cerrno>
#include "net_socket.hh"

namespace sel::net {

namespace error {
  enum Errors {
    ACCESS_DENIED           = SOCKET_ERR(EACCES),
    ADDRESS_IN_USE          = SOCKET_ERR(EADDRINUSE),
    ALREADY_CONNECTED       = SOCKET_ERR(EISCONN),
    ALREADY_STARTED         = SOCKET_ERR(EALREADY),
    CONNECTION_ABORTED      = SOCKET_ERR(ECONNABORTED),
    CONNECTION_REFUSED      = SOCKET_ERR(ECONNREFUSED),
    CONNECTION_RESET        = SOCKET_ERR(ECONNRESET),
    BAD_DESCRIPTOR          = SOCKET_ERR(EBADF),
    FAULT                   = SOCKET_ERR(EFAULT),
    HOST_UNREACHABLE        = SOCKET_ERR(EHOSTUNREACH),
    IN_PROGRESS             = SOCKET_ERR(EINPROGRESS),
    INTERRUPTED             = SOCKET_ERR(EINTR),
    INVALID_ARGUMENT        = SOCKET_ERR(EINVAL),
    MESSAGE_SIZE            = SOCKET_ERR(EMSGSIZE),
    NAME_TOO_LONG           = SOCKET_ERR(ENAMETOOLONG),
    NETWORK_DOWN            = SOCKET_ERR(ENETDOWN),
    NETWORK_RESET           = SOCKET_ERR(ENETRESET),
    NETWORK_UNREACHABLE     = SOCKET_ERR(ENETUNREACH),
    NO_DESCRIPTOR           = SOCKET_ERR(EMFILE),
    NO_BUFFER               = SOCKET_ERR(ENOBUFS),
    NO_MEMORY               = WIN_OR_POSIX(NATIVE_ERR(ERROR_OUTOFMEMORY), NATIVE_ERR(ENOMEM)),
    NO_PERMISSION           = WIN_OR_POSIX(NATIVE_ERR(ERROR_ACCESS_DENIED), NATIVE_ERR(EPERM)),
    NO_PROTOCAL_OPTION      = SOCKET_ERR(ENOPROTOOPT),
    NO_SUCH_DEVIVE          = WIN_OR_POSIX(NATIVE_ERR(ERROR_BAD_UNIT), NATIVE_ERR(ENODEV)),
    NOT_CONNECTED           = SOCKET_ERR(ENOTCONN),
    NOT_SOCKET              = SOCKET_ERR(ENOTSOCK),
    OPERATION_ABORTED       = WIN_OR_POSIX(NATIVE_ERR(ERROR_OPERATION_ABORTED), NATIVE_ERR(ECANCELED)),
    OPERATION_NOT_SUPPORTED = SOCKET_ERR(EOPNOTSUPP),
    SHUTDOWN                = SOCKET_ERR(ESHUTDOWN),
    TIMED_OUT               = SOCKET_ERR(ETIMEDOUT),
    TRYAGAIN                = WIN_OR_POSIX(NATIVE_ERR(ERROR_RETRY), NATIVE_ERR(EAGAIN)),
    WOULDBLOCK              = NATIVE_ERR(EWOULDBLOCK),
  };
}

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

void Socket::setblocking(bool flag) {}
bool Socket::listen(strv_t host, u16_t port, int backlog) { return false; }
bool Socket::connect(strv_t host, u16_t port) { return false; }

}