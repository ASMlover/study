// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_NET_ENDIAN_HEADER_H__
#define __TYR_NET_ENDIAN_HEADER_H__

#include <stdint.h>
#include "../basic/TPlatform.h"
#if defined(TYR_WINDOWS)
# include <WinSock2.h>

# define TYR_LITTLE_ENDIAN 0x41424344UL
# define TYR_BIG_ENDIAN 0x44434241UL
# define TYR_PDP_ENDIAN 0x42414443UL
# define TYR_BYTE_ORDER ('ABCD')

# if TYR_BYTE_ORDER == TYR_LITTLE_ENDIAN
#   define htobe16(x) htons((x))
#   define htole16(x) (x)
#   define be16toh(x) ntohs((x))
#   define le16toh(x) (x)
#   define htobe32(x) htonl((x))
#   define htole32(x) (x)
#   define be32toh(x) ntohl((x))
#   define le32toh(x) (x)
#   define htobe64(x) htonll((x))
#   define htole64(x) (x)
#   define be64toh(x) ntohll((x))
#   define le64toh(x) (x)
# elif TYR_BYTE_ORDER == TYR_BIG_ENDIAN
#   define htobe16(x) (x)
#   define htole16(x) __builtin_bswap16((x))
#   define be16toh(x) (x)
#   define le16toh(x) __builtin_bswap16((x))
#   define htobe32(x) (x)
#   define htole32(x) __builtin_bswap32((x))
#   define be32toh(x) (x)
#   define le32toh(x) __builtin_bswap32((x))
#   define htobe64(x) (x)
#   define htole64(x) __builtin_bswap64((x))
#   define be64toh(x) (x)
#   define le64toh(x) __builtin_bswap64((x))
# else
#   error "byte order not supported"
# endif
#elif defined(TYR_LINUX)
# include <endian.h>
#elif defined(TYR_DARWIN)
# include <libkern/OSByteOrder.h>

# define htobe16(x) OSSwapHostToBigInt16((x))
# define htole16(x) OSSwapHostToLittleInt16((x))
# define be16toh(x) OSSwapBigToHostInt16((x))
# define le16toh(x) OSSwapLittleToHostInt16((x))
# define htobe32(x) OSSwapHostToBigInt32((x))
# define htole32(x) OSSwapHostToLittleInt32((x))
# define be32toh(x) OSSwapBigToHostInt32((x))
# define le32toh(x) OSSwapLittleToHostInt32((x))
# define htobe64(x) OSSwapHostToBigInt64((x))
# define htole64(x) OSSwapHostToLittleInt64((x))
# define be64toh(x) OSSwapBigToHostInt64((x))
# define le64toh(x) OSSwapLittleToHostInt64((x))
#endif

namespace tyr { namespace net {

inline uint16_t host_to_net16(uint16_t host16) {
  return htobe16(host16);
}

inline uint32_t host_to_net32(uint32_t host32) {
  return htobe32(host32);
}

inline uint64_t host_to_net64(uint64_t host64) {
  return htobe64(host64);
}

inline uint16_t net_to_host16(uint16_t net16) {
  return be16toh(net16);
}

inline uint32_t net_to_host32(uint32_t net32) {
  return be32toh(net32);
}

inline uint64_t net_to_host64(uint64_t net64) {
  return be64toh(net64);
}

}}

#endif // __TYR_NET_ENDIAN_HEADER_H__
