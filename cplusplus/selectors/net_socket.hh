#pragma once

#include <system_error>
#include "net_types.hh"

namespace sel::net {

class Socket : public Copyable {
  socket_t sockfd_{kINVALID};
public:
  Socket() noexcept {}
  Socket(socket_t sockfd) noexcept : sockfd_(sockfd) {}

  inline socket_t sockfd() const noexcept { return sockfd_; }
  inline operator socket_t() const noexcept { return sockfd_; }
  inline bool is_valid() const noexcept { return sockfd_ != kINVALID; }
  inline operator bool() const noexcept { return sockfd_ != kINVALID; }

  void open();
  void open(std::error_code& ec);
  void close();
  void close(std::error_code& ec);

  void setblocking(bool flag);

  bool listen(strv_t host = "0.0.0.0", u16_t port = 5555, int backlog = 5);
  bool connect(strv_t host = "127.0.0.1", u16_t port = 5555);
};

}