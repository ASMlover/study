#pragma once

#include "../common/common.hh"
#if defined(SSER_NIX)
# include <sys/types.h>
# include <sys/socket.h>
#endif

namespace sser::net {

#if defined(SSER_WIN)
# if defined(SSER_ARCH64)
  using socket_t    = u64_t;
# else
  using socket_t    = u32_t;
# endif
  using socklen_t   = int;
  using sa_family_t = int;
  using in_addr_t   = u32_t;
#else
  using socket_t    = int;
#endif

static constexpr socket_t kINVALID  = (socket_t)(~0);
static constexpr int      kERROR    = -1;

}