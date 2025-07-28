// Copyright (c) 2024 ASMlover. All rights reserved.
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
#pragma once

#include "common.hh"

#if defined(EV_WINDOWS)
# include <Windows.h>
# include <WinSock2.h>
# include <ws2tcpip.h>
# include <ws2def.h>
# include <io.h>
# include <ws2ipdef.h>

  using socket_t                  = SOCKET;
  using ev_socket_t               = intptr_t;
  using iovec                     = _WSABUF;
# define iov_base                 buf
# define iov_len                  len

# define EWOULDBLOCK              WSAEWOULDBLOCK
# define EINPROGRESS              WSAEINPROGRESS
# define EALREADY                 WSAEALREADY
# define ENOTSOCK                 WSAENOTSOCK
# define EDESTADDRREQ             WSAEDESTADDRREQ
# define EMSGSIZE                 WSAEMSGSIZE
# define EPROTOTYPE               WSAEPROTOTYPE
# define ENOPROTOOPT              WSAENOPROTOOPT
# define EPROTONOSUPPORT          WSAEPROTONOSUPPORT
# define ESOCKTNOSUPPORT          WSAESOCKTNOSUPPORT
# define EOPNOTSUPP               WSAEOPNOTSUPP
# define EPFNOSUPPORT             WSAEPFNOSUPPORT
# define EAFNOSUPPORT             WSAEAFNOSUPPORT
# define EADDRINUSE               WSAEADDRINUSE
# define EADDRNOTAVAIL            WSAEADDRNOTAVAIL
# define ENETDOWN                 WSAENETDOWN
# define ENETUNREACH              WSAENETUNREACH
# define ENETRESET                WSAENETRESET
# define ECONNABORTED             WSAECONNABORTED
# define ECONNRESET               WSAECONNRESET
# define ENOBUFS                  WSAENOBUFS
# define EISCONN                  WSAEISCONN
# define ENOTCONN                 WSAENOTCONN
# define ESHUTDOWN                WSAESHUTDOWN
# define ETOOMANYREFS             WSAETOOMANYREFS
# define ETIMEDOUT                WSAETIMEDOUT
# define ECONNREFUSED             WSAECONNREFUSED
# define ELOOP                    WSAELOOP
# define ENAMETOOLONG             WSAENAMETOOLONG
# define EHOSTDOWN                WSAEHOSTDOWN
# define EHOSTUNREACH             WSAEHOSTUNREACH
# define ENOTEMPTY                WSAENOTEMPTY
# define EPROCLIM                 WSAEPROCLIM
# define EUSERS                   WSAEUSERS
# define EDQUOT                   WSAEDQUOT
# define ESTALE                   WSAESTALE
# define EREMOTE                  WSAEREMOTE

# define EAGAIN                   EWOULDBLOCK
# define gai_strerror             gai_strerrorA

# if !defined(HAVE_MSG_NOSIGNAL)
#   if !defined(MSG_NOSIGNAL)
#     define MSG_NOSIGNAL         (0)
#   endif
# endif

# if !defined(HAVE_MSG_DONTWAIT)
#   if !defined(MSG_DONTWAIT)
#     define MSG_DONTWAIT         (0)
#   endif
# endif

# define EVUTIL_ERR_RW_RETRIABLE(e)\
    ((e) == WSAEWOULDBLOCK || (e) == WSAETIMEDOUT || (e) == WSAEINTR)

# define EVUTIL_ERR_CONNECT_RETRIABLE(e)\
    ((e) == WSAEWOULDBLOCK || (e) == WSAEINTR || (e) == WSAEINPROGRESS || (e) == WSAEINVAL)

# define EVUTIL_ERR_ACCEPT_RETRIABLE(e)     EVUTIL_ERR_RW_RETRIABLE(e)

# define EVUTIL_ERR_CONNECT_REFUSED(e)      ((e) == WSAECONNREFUSED)

int readv(socket_t sockfd, struct iovec* iov, int iovcnt);
#else
# include <unistd.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# include <sys/uio.h>
# include <arpa/inet.h>

  using socket_t                  = int;
  using ev_socket_t               = int;

# if !defined(INVALID_SOCKET)
#   define INVALID_SOCKET         (-1)
# endif

# define EVUTIL_ERR_RW_RETRIABLE(e)         ((e) == EINTR || (e) == EAGAIN)
# define EVUTIL_ERR_CONNECT_RETRIABLE(e)    ((e) == EINTR || (e) == EINPROGRESS)
# define EVUTIL_ERR_ACCEPT_RETRIABLE(e)     ((e) == EINTR || (e) == EAGAIN || (e) == ECONNABORTED)
# define EVUTIL_ERR_CONNECT_REFUSED(e)      ((e) == ECONNREFUSED)
#endif

using signal_number_t             = ev_socket_t;
