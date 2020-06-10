#pragma once

#if defined(SEL_WIN)
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif
#include "common.hh"

namespace sel::net {

enum class Event {
  READ  = 0x01,
  WRITE = 0x02,
};

class Socket;

class Selector final : private UnCopyable {
  fd_set rfds_master_;
  fd_set wfds_master_;
public:
  void reg(Socket& s, Event events);
  void unreg(Socket& s);
};

}