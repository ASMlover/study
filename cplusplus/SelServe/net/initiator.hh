#pragma once

#if defined(SSER_WIN)
# include <WinSock2.h>
#endif
#include "../common/common.hh"

namespace sser::net {

template <int Major = 2, int Minor = 2>
class Initiator final : private UnCopyable {
#if defined(SSER_WIN)
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
  Initiator() { startup(Major, Minor); }
  ~Initiator() { cleanup(); }
#endif
};

}