#pragma once

#if defined(SEL_WIN)
# include <WinSock2.h>
#endif
#include "common.hh"

namespace sel::net {

template <int Major = 2, int Minor = 2>
class Initializer final : private UnCopyable {
#if defined(SEL_WIN)
  long init_count_{};

  void startup(int major, int minor) {
    if (::InterlockedIncrement(&init_count_) == 1L) {
      WSADATA wd;
      ::WSAStartup(MAKEWORD(major, minor), &wd);
    }
  }

  void cleanup() {
    if (::InterlockedDecrement(&init_count_) == 0L)
      ::WSACleanup();
  }
public:
  Initializer() noexcept { startup(Major, Minor); }
  ~Initializer() noexcept { cleanup(); }
#endif
};

}