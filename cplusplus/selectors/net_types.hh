#pragma once

#include "common.hh"
#if defined(SEL_LINUX)
# include <sys/types.h>
# include <sys/socket.h>
#endif

namespace sel::net {

#if defined(SEL_WIN)
# if defined(SEL_ARCH64)
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