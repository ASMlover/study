#pragma once

#include <WinSock2.h>
#include "common.hh"

namespace coext {

using socket_t = SOCKET;

class WSGuarder final : private UnCopyable {
public:
  WSGuarder() noexcept {
    WSADATA wd;
    ::WSAStartup(MAKEWORD(2, 2), &wd);
  }

  ~WSGuarder() noexcept {
    ::WSACleanup();
  }
};

enum class NetOption {
  OPTION_NONE,  // do nothing
  OPTION_AGAIN, // do it again (in non-blocking)
};

// only for tcp protocal
class Socket : public Copyable {
  socket_t fd_{ INVALID_SOCKET };
public:
  Socket() noexcept {}
  Socket(socket_t fd) noexcept : fd_(fd) {}

  inline operator bool() const noexcept { return is_valid(); }
  inline bool is_valid() const noexcept { return fd_ != INVALID_SOCKET; }
  inline operator socket_t() const noexcept { return get(); }
  inline socket_t get() const noexcept { return fd_; }

  bool open();
  void close();

  void set_nonblocking(bool mode = true);

  bool bind(strv_t host, u16_t port = 5555);
  bool listen(int backlog = 5);

  bool connect(strv_t host, u16_t port = 5555);
  std::tuple<bool, NetOption> connect_async(strv_t host, u16_t port = 5555);
  std::tuple<bool, Socket> accept();
  std::tuple<bool, Socket, NetOption> accept_async();
  std::tuple<bool, sz_t> read(char* buf, sz_t len);
  std::tuple<bool, sz_t, NetOption> read_async(char* buf, sz_t len);
  std::tuple<bool, sz_t> write(const char* buf, sz_t len);
  std::tuple<bool, sz_t, NetOption> write_async(const char* buf, sz_t len);
};

}