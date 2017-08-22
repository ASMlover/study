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
#include "Types.h"
#if defined(KCPNET_WINDOWS)
# include <Windows.h>
#else
# include <sys/time.h>
#endif
#include <cstring>
#include "Utility.h"

namespace KcpNet {

static const char* kKcpNetConnectRequest = "kcpnet.connect.request";
static constexpr std::size_t kKcpNetConnectRequestLen = 22;
static const char* kKcpNetConnectResponse = "kcpnet.connect.response";
static constexpr std::size_t kKcpNetConnectResponseLen = 23;

#if defined(KCPNET_WINDOWS)
static int gettimeofday(struct timeval* tv, struct timezone* /*tz*/) {
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

std::uint32_t get_clock32(void) {
  return static_cast<std::uint32_t>(get_clock64() & 0xFFFFFFFF);
}

std::uint64_t get_clock64(void) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<std::uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

std::string make_connect_request(void) {
  return std::string(kKcpNetConnectRequest);
}

bool is_connect_request(const char* buf, std::size_t len) {
  return (len == kKcpNetConnectRequestLen) &&
    (std::memcmp(buf, kKcpNetConnectRequest, kKcpNetConnectRequestLen) == 0);
}

std::string make_connect_response(std::uint32_t conv) {
  char buf[256];
  std::snprintf(buf, sizeof(buf), "%s %u", kKcpNetConnectResponse, conv);
  return std::string(buf);
}

bool is_connect_response(const char* buf, std::size_t len) {
  return (len > kKcpNetConnectResponseLen) &&
    (std::memcmp(buf, kKcpNetConnectResponse, kKcpNetConnectResponseLen) == 0);
}

std::uint32_t get_conv_from_connect_response(const char* buf) {
  return std::atol(buf + kKcpNetConnectResponseLen);
}

}
