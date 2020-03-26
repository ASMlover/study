#pragma once

#include <optional>
#include "common_utils.hh"

namespace svrutils {

class Socket : public common::Copyable {
  SOCKET fd_{INVALID_SOCKET};
public:
  Socket(SOCKET fd) noexcept : fd_(fd) {}

  inline operator SOCKET() const noexcept { return fd_; }
  inline operator bool() const noexcept { return fd_ == INVALID_SOCKET; }
  inline SOCKET get() const noexcept { return fd_; }
  inline void reset(SOCKET fd) noexcept { fd_ = fd; }

  bool open();
  void close();
  void set_nonblocking(bool mode = true);

  bool bind(std::string_view addr, std::uint16_t port = 5555);
  bool listen(int backlog = 5);

  bool connect(std::string_view addr, std::uint16_t port = 5555);
  std::tuple<bool, bool> connect_async(std::string_view addr, std::uint16_t port = 5555);
  std::tuple<bool, Socket> accept();
  std::tuple<bool, Socket> accept_async();
  std::tuple<bool, std::size_t> read(char* buf, std::size_t len);
  std::tuple<bool, std::size_t> read_async(char* buf, std::size_t len);
  std::tuple<bool, std::size_t> write(const char* buf, std::size_t len);
  std::tuple<bool, std::size_t> write_async(const char* buf, std::size_t len);
};

std::optional<SOCKET> create_server(std::uint16_t port = 5555);
void set_nonblocking(common::UniqueSocket& sockfd);
void serve_connection(common::UniqueSocket& sockfd);

}