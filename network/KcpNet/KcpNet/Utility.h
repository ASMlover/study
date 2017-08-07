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
#pragma once

#if defined(_WINDOWS_) || defined(_MSC_VER)
# include <Windows.h>
#else
# include <sys/time.h>
#endif
#include <cstring>
#include <string>

namespace KcpNet {

#if defined(_WINDOWS_) || defined(_MSC_VER)
inline int gettimeofday(struct timeval* tv, struct timezone* /*tz*/) {
  if (tv) {
    FILETIME ft;
    SYSTEMTIME st;
    ULARGE_INTEGER uli;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    tv->tv_sec = static_cast<long>(
        (uli.QuadPart - 116444736000000000ULL) / 10000000L);
    tv->tv_usec = static_cast<long>(st.wMilliseconds * 1000);
  }
  return 0;
}
#endif

inline std::uint64_t get_clock64(void) {
  struct timeval t;
  gettimeofday(&t, nullptr);
  return ((std::uint64_t)t.tv_sec) * 1000 + (t.tv_usec / 1000);
}

inline std::uint32_t get_clock32(void) {
  return (std::uint32_t)(get_clock64() & 0xFFFFFFFF);
}

#define KCPNET_CONNECT_PACKET "kcpnet-connect-packet"
#define KCPNET_SENDBACK_PACKET "kcpnet-sendback-packet"

inline std::string make_connect_packet(void) {
  return std::string(KCPNET_CONNECT_PACKET, sizeof(KCPNET_CONNECT_PACKET));
}

inline bool is_connect_packet(const char* buf, std::size_t len) {
  return (len == sizeof(KCPNET_CONNECT_PACKET) &&
      std::memcmp(buf,
        KCPNET_CONNECT_PACKET, sizeof(KCPNET_CONNECT_PACKET) - 1) == 0);
}

inline std::string make_sendback_packet(std::uint32_t conv) {
  char buf[256]{};
  std::size_t n = std::snprintf(buf,
      sizeof(buf), "%s %u", KCPNET_SENDBACK_PACKET, conv);
  return std::string(buf, n);
}

}
