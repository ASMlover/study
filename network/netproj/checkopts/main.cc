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
#include <cstdio>
#include <iostream>
#include <Chaos/Base/Platform.h>
#include <Chaos/Base/Types.h>
#include "../libase/netops.h"

union Val {
  int i_val;
  long l_val;
  struct linger linger_val;
  struct timeval timeval_val;
} val;

static const char* sock_str_flag(Val* v, int n);
static const char* sock_str_int(Val* v, int n);
static const char* sock_str_linger(Val* v, int n);
static const char* sock_str_timeval(Val* v, int n);

struct SockOpts {
  const char* opt_str;
  int opt_level;
  int opt_name;
  const char* (*opt_fn)(Val*, int);
} sock_opts[] = {
  {"SO_BROADCAST", SOL_SOCKET, SO_BROADCAST, sock_str_flag},
  {"SO_DEBUG", SOL_SOCKET, SO_DEBUG, sock_str_flag},
  {"SO_DONTROUTE", SOL_SOCKET, SO_DONTROUTE, sock_str_flag},
  {"SO_ERROR", SOL_SOCKET, SO_ERROR, sock_str_int},
  {"SO_KEEPALIVE", SOL_SOCKET, SO_KEEPALIVE, sock_str_flag},
  {"SO_LINGER", SOL_SOCKET, SO_LINGER, sock_str_linger},
  {"SO_OOBINLINE", SOL_SOCKET, SO_OOBINLINE, sock_str_flag},
  {"SO_RCVBUF", SOL_SOCKET, SO_RCVBUF, sock_str_int},
  {"SO_SNDBUF", SOL_SOCKET, SO_SNDBUF, sock_str_int},
  {"SO_RCVLOWAT", SOL_SOCKET, SO_RCVLOWAT, sock_str_int},
  {"SO_SNDLOWAT", SOL_SOCKET, SO_SNDLOWAT, sock_str_int},
  {"SO_RCVTIMEO", SOL_SOCKET, SO_RCVTIMEO, sock_str_timeval},
  {"SO_SNDTIMEO", SOL_SOCKET, SO_SNDTIMEO, sock_str_timeval},
#if defined(SO_REUSEPORT)
  {"SO_REUSEPORT", SOL_SOCKET, SO_REUSEPORT, sock_str_flag},
#else
  {"SO_REUSEPORT", 0, 0, nullptr},
#endif
  {"SO_TYPE", SOL_SOCKET, SO_TYPE, sock_str_int},
#if defined(SO_USELOOPBACK)
  {"SO_USELOOPBACK", SOL_SOCKET, SO_USELOOPBACK, sock_str_flag},
#else
  {"SO_USELOOPBACK", 0, 0, nullptr},
#endif
  {"IP_TOS", IPPROTO_IP, IP_TOS, sock_str_int},
  {"IP_TTL", IPPROTO_IP, IP_TTL, sock_str_int},
#if defined(IPV6_DONTFRAG)
  {"IPV6_DONTFRAG", IPPROTO_IPV6, IPV6_DONTFRAG, sock_str_flag},
#else
  {"IPV6_DONTFRAG", 0, 0, nullptr},
#endif
  {"IPV6_UNICAST_HOPS", IPPROTO_IPV6, IPV6_UNICAST_HOPS, sock_str_int},
  {"IPV6_V6ONLY", IPPROTO_IPV6, IPV6_V6ONLY, sock_str_flag},
  {"TCP_MAXSEG", IPPROTO_TCP, TCP_MAXSEG, sock_str_int},
  {"TCP_NODELAY", IPPROTO_TCP, TCP_NODELAY, sock_str_flag},
#if defined(SCTP_AUTOCLOSE)
  {"SCTP_AUTOCLOSE", IPPROTO_SCTP, SCTP_AUTOCLOSE, sock_str_int},
#else
  {"SCTP_AUTOCLOSE", 0, 0, nullptr},
#endif
#if defined(SCTP_MAXBURSET)
  {"SCTP_MAXBURSET", IPPROTO_SCTP, SCTP_MAXBURSET, sock_str_int},
#else
  {"SCTP_MAXBURSET", 0, 0, nullptr},
#endif
#if defined(SCTP_MAXSEG)
  {"SCTP_MAXSEG", IPPROTO_SCTP, SCTP_MAXSEG, sock_str_int},
#else
  {"SCTP_MAXSEG", 0, 0, nullptr},
#endif
#if defined(SCTP_NODELAY)
  {"SCTP_NODELAY", IPPROTO_SCTP, SCTP_NODELAY, sock_str_flag},
#else
  {"SCTP_NODELAY", 0, 0, nullptr},
#endif
  {nullptr, 0, 0, nullptr},
};

static char s_buf[1024];

static const char* sock_str_flag(Val* v, int n) {
  if (n != sizeof(int))
    std::snprintf(s_buf, sizeof(s_buf), "size (%d) not sizeof(int)", n);
  else
    std::snprintf(s_buf, sizeof(s_buf), "%s", v->i_val == 0 ? "off" : "on");
  return s_buf;
}

static const char* sock_str_int(Val* v, int n) {
  return sock_str_flag(v, n);
}

static const char* sock_str_linger(Val* v, int n) {
  if (n != sizeof(struct linger))
    std::snprintf(s_buf, sizeof(s_buf), "size (%d) not sizeof(linger)", n);
  else
    std::snprintf(s_buf, sizeof(s_buf), "l_onoff=%d, l_linger=%d",
        v->linger_val.l_onoff, v->linger_val.l_linger);
  return s_buf;
}

static const char* sock_str_timeval(Val* v, int n) {
  if (n != sizeof(struct timeval))
    std::snprintf(s_buf, sizeof(s_buf), "size (%d) not sizeof(timeval)", n);
  else
    std::snprintf(s_buf, sizeof(s_buf), "sec=%ld, usec=%ld",
        v->timeval_val.tv_sec, v->timeval_val.tv_usec);
  return s_buf;
}

int main(int argc, char* argv[]) {
  CHAOS_UNUSED(argc), CHAOS_UNUSED(argv);

  net::startup();

  int sockfd;
  for (auto* opt = sock_opts; opt->opt_str != nullptr; ++opt) {
    std::cout << opt->opt_str << " - ";
    if (opt->opt_fn == nullptr) {
      std::cout << "(undefined)" << std::endl;
    }
    else {
      switch (opt->opt_level) {
      case SOL_SOCKET:
      case IPPROTO_IP:
      case IPPROTO_TCP:
        sockfd = net::socket::open(AF_INET, SOCK_STREAM, 0);
        break;
#if defined(IPPROTO_IPV6)
      case IPPROTO_IPV6:
        sockfd = net::socket::open(AF_INET6, SOCK_STREAM, 0);
        break;
#endif
#if defined(IPPROTO_SCTP)
      case IPPROTO_SCTP:
        sockfd = net::socket::open(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        break;
#endif
      default:
        exit(-1);
        break;
      }

      socklen_t n = sizeof(val);
      if (net::socket::get_option(sockfd,
            opt->opt_level, opt->opt_name, &val, &n) == -1) {
        std::cerr << "getsockopt error" << std::endl;
        exit(-1);
      }
      else {
        std::cout << "default = " << opt->opt_fn(&val, n) << std::endl;
      }
      net::socket::close(sockfd);
    }
  }

  net::cleanup();

  return 0;
}
