#pragma once

#if defined(SSER_WIN)
# include <winerror.h>
# include <WinSock2.h>

# define NATIVE_ERR(E)              E
# define SOCKET_ERR(E)              WSA##E
# define COMPATIBLE_ERR(WinE, NixE) WinE
#else
#include <cerrno>

# define NATIVE_ERR(E)              E
# define SOCKET_ERR(E)              E
# define COMPATIBLE_ERR(WinE, NixE) NixE
#endif
#include <system_error>
#include "../common/common.hh"

namespace sser::net::error {

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
  NO_MEMORY               = COMPATIBLE_ERR(NATIVE_ERR(ERROR_OUTOFMEMORY), NATIVE_ERR(ENOMEM)),
  NO_PERMISSION           = COMPATIBLE_ERR(NATIVE_ERR(ERROR_ACCESS_DENIED), NATIVE_ERR(EPERM)),
  NO_PROTOCOL_OPTION      = SOCKET_ERR(ENOPROTOOPT),
  NO_SUCH_DEVICE          = COMPATIBLE_ERR(NATIVE_ERR(ERROR_BAD_UNIT), NATIVE_ERR(ENODEV)),
  NOT_CONNECTED           = SOCKET_ERR(ENOTCONN),
  NOT_SOCKET              = SOCKET_ERR(ENOTSOCK),
  OPERATION_ABORTED       = COMPATIBLE_ERR(NATIVE_ERR(ERROR_OPERATION_ABORTED), NATIVE_ERR(ECANCELED)),
  OPERATION_NOT_SUPPORTED = SOCKET_ERR(EOPNOTSUPP),
  SHUTDOWN                = SOCKET_ERR(ESHUTDOWN),
  TIMED_OUT               = SOCKET_ERR(ETIMEDOUT),
  TRYAGAIN                = COMPATIBLE_ERR(NATIVE_ERR(ERROR_RETRY), NATIVE_ERR(EAGAIN)),
  WOULDBLOCK              = SOCKET_ERR(EWOULDBLOCK),
};

inline void clear_errno() noexcept {
#if defined(SSER_WIN)
  ::WSASetLastError(0);
#else
  errno = 0;
#endif
}

inline int get_errno() noexcept {
#if defined(SSER_WIN)
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

inline std::error_code make_err(int ec) noexcept {
  return std::error_code(ec, std::system_category());
}

template <typename Ret>
inline Ret wrap(Ret r, std::error_code& ec) noexcept {
  ec = make_err(get_errno());
  return r;
}

inline void throw_err(const std::error_code& ec) {
  if (ec)
    throw std::system_error(ec);
}

inline void throw_err(const std::error_code& ec, const char* location) {
  if (ec)
    throw std::system_error(ec, string_format("%s: %s", location, ec.message().c_str()));
}

}