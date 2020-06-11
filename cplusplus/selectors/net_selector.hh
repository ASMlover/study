#pragma once

#if defined(SEL_WIN)
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif
#include <functional>
#include <unordered_map>
#include "common.hh"

namespace sel::net {

enum class Event : u8_t {
  NONE  = 0x00,
  READ  = 0x01,
  WRITE = 0x02,
};

inline bool operator|(Event a, Event b) noexcept {
  return as_type<bool>(as_type<u8_t>(a) | as_type<u8_t>(b));
}

inline Event operator|=(Event a, Event b) noexcept {
  a = as_type<Event>(as_type<u8_t>(a) | as_type<u8_t>(b));
  return a;
}

inline bool operator&(Event a, Event b) noexcept {
  return as_type<bool>(as_type<u8_t>(a) & as_type<u8_t>(b));
}

inline Event operator&=(Event a, Event b) noexcept {
  a = as_type<Event>(as_type<u8_t>(a) & as_type<u8_t>(b));
  return a;
}

class Socket;
using SocketPtr = std::shared_ptr<Socket>;
using EventFn   = std::function<void(SocketPtr&, Event)>;
using EventItem = std::tuple<SocketPtr, Event, EventFn>;

class Selector final : private UnCopyable {
  using SocketEntry = std::pair<SocketPtr, EventFn>;

  fd_set rfds_master_;
  fd_set wfds_master_;

  int max_sockfd_{-1};
  std::unordered_map<socket_t, SocketEntry> sockets_;
public:
  Selector() noexcept;
  ~Selector();

  void reg(const SocketPtr& s, Event events, EventFn&& fn);
  void unreg(const SocketPtr& s);
  std::vector<EventItem> select();
};

}