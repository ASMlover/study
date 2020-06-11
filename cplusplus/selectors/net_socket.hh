#pragma once

#include <system_error>
#include "net_types.hh"
#include "net_buffer.hh"

namespace sel::net {

class Socket : public Copyable {
  socket_t sockfd_{kINVALID};
  bool blocking_{true};
public:
  Socket() noexcept {}
  Socket(socket_t sockfd) noexcept : sockfd_(sockfd) {}
  Socket(socket_t sockfd, bool is_blocking) noexcept : sockfd_(sockfd), blocking_(is_blocking) {}

  inline socket_t sockfd() const noexcept { return sockfd_; }
  inline operator socket_t() const noexcept { return sockfd_; }
  inline bool is_valid() const noexcept { return sockfd_ != kINVALID; }
  inline operator bool() const noexcept { return sockfd_ != kINVALID; }
  inline bool is_blocking() const noexcept { return blocking_; }

  void open();
  void open(std::error_code& ec);
  void close();
  void close(std::error_code& ec);

  void setblocking(bool is_blocking);
  void setblocking(bool is_blocking, std::error_code& ec);

  void listen(strv_t host = "0.0.0.0", u16_t port = 5555);
  void listen(strv_t host, u16_t port, std::error_code& ec);
  void connect(strv_t host = "127.0.0.1", u16_t port = 5555);
  void connect(strv_t host, u16_t port, std::error_code& ec);
  socket_t accept();
  socket_t accept(std::error_code& ec);
  sz_t read(const MutableBuf& buf);
  sz_t read(const MutableBuf& buf, std::error_code& ec);
  sz_t read_some(const MutableBuf& buf);
  sz_t read_some(const MutableBuf& buf, std::error_code& ec);
  sz_t write(const ConstBuf& buf);
  sz_t write(const ConstBuf& buf, std::error_code& ec);
  sz_t write_some(const ConstBuf& buf);
  sz_t write_some(const ConstBuf& buf, std::error_code& ec);
};

template <typename... Args>
inline auto make_socket(Args&&... args) noexcept {
  return std::make_shared<Socket>(std::forward<Args>(args)...);
}

}