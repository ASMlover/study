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
#ifndef NEPTUNE_ENDIAN_H
#define NEPTUNE_ENDIAN_H

#include <cstdint>
#include <Chaos/Platform.h>

#if defined(CHAOS_WINDOWS)
# if CHAOS_BYTE_ORDER == CHAOS_LITTLE_ENDIAN
#   if !defined(_WINDOWS_)
#     include <WinSock2.h>
#   endif

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
# elif CHAOS_BYTE_ORDER == CHAOS_BIG_ENDIAN
#   include <cstdlib>

#   define htobe16(x) (x)
#   define htole16(x) _byteswap_ushort((x))
#   define be16toh(x) (x)
#   define le16toh(x) _byteswap_ushort((x))

#   define htobe32(x) (x)
#   define htole32(x) _byteswap_ulong((x))
#   define be32toh(x) (x)
#   define le32toh(x) _byteswap_ulong((x))

#   define htobe64(x) (x)
#   define htole64(x) _byteswap_uint64((x))
#   define be64toh(x) (x)
#   define le64toh(x) _byteswap_uint64((x))
# else
#   error "byte order not supported"
# endif
#elif defined(CHAOS_LINUX)
# include <endian.h>
#elif defined(CHAOS_DARWIN)
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

namespace Neptune {

inline std::uint16_t h2n16(std::uint16_t h16) {
  return htobe16(h16);
}

inline std::uint16_t n2h16(std::uint16_t n16) {
  return be16toh(n16);
}

inline std::uint32_t h2n32(std::uint32_t h32) {
  return htobe32(h32);
}

inline std::uint32_t n2h32(std::uint32_t n32) {
  return be32toh(n32);
}

inline std::uint64_t h2n64(std::uint64_t h64) {
  return htobe64(h64);
}

inline std::uint64_t n2h64(std::uint64_t n64) {
  return be64toh(n64);
}

}

#endif // NEPTUNE_ENDIAN_H
