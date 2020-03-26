#include "server_utils.hh"

namespace svrutils {

enum class NetStatus {
  MSG_WAIT,
  MSG_RECV,
};

bool Socket::open() {
  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return fd_ != INVALID_SOCKET;
}

void Socket::close() {
  if (fd_ != INVALID_SOCKET) {
    ::shutdown(fd_, SD_BOTH);
    ::closesocket(fd_);
    fd_ = INVALID_SOCKET;
  }
}

void Socket::set_nonblocking(bool mode) {
  u_long flag = mode ? 1 : 0;
  ::ioctlsocket(fd_, FIONBIO, &flag);
}

bool Socket::bind(std::string_view addr, std::uint16_t port) {
  sockaddr_in bind_addr;
  bind_addr.sin_addr.s_addr = ::inet_addr(addr.data());
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_port = ::htons(port);

  return ::bind(fd_, (const sockaddr*)&bind_addr, static_cast<int>(sizeof(bind_addr))) != SOCKET_ERROR;
}

bool Socket::listen(int backlog) {
  return ::listen(fd_, backlog) != SOCKET_ERROR;
}

bool Socket::connect(std::string_view addr, std::uint16_t port) {
  sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(addr.data());
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(port);

  return ::connect(fd_, (const sockaddr*)&remote_addr, static_cast<int>(sizeof(remote_addr))) != SOCKET_ERROR;
}

std::tuple<bool, bool> Socket::connect_async(std::string_view addr, std::uint16_t port) {
  // return (success:bool, connect_again:bool)

  if (!connect(addr, port)) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(true, true);
    return std::make_tuple(false, false);
  }
  return std::make_tuple(true, false);
}

std::tuple<bool, Socket> Socket::accept() {
  sockaddr_in peer_addr;
  int peer_addr_len = static_cast<int>(sizeof(peer_addr));

  SOCKET fd = ::accept(fd_, (sockaddr*)&peer_addr, &peer_addr_len);
  return std::make_tuple(fd != INVALID_SOCKET, fd);
}

std::tuple<bool, Socket> Socket::accept_async() {
  sockaddr_in peer_addr;
  int peer_addr_len = static_cast<int>(sizeof(peer_addr));

  SOCKET fd = ::accept(fd_, (sockaddr*)&peer_addr, &peer_addr_len);
  if (fd == INVALID_SOCKET && WSAGetLastError() != WSAEWOULDBLOCK)
    return std::make_tuple(false, fd);
  return std::make_tuple(true, fd);
}

std::tuple<bool, std::size_t> Socket::read(char* buf, std::size_t len) {
  int rlen = ::recv(fd_, buf, static_cast<int>(len), 0);
  return std::make_tuple(rlen > 0, rlen);
}

std::tuple<bool, std::size_t> Socket::read_async(char* buf, std::size_t len) {
  int rlen = ::recv(fd_, buf, static_cast<int>(len), 0);
  if (rlen < 0) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(true, 0);
    return std::make_tuple(false, 0);
  }
  else if (rlen == 0) {
    return std::make_tuple(false, 0);
  }
  return std::make_tuple(true, rlen);
}

std::tuple<bool, std::size_t> Socket::write(const char* buf, std::size_t len) {
  int wlen = ::send(fd_, buf, static_cast<int>(len), 0);
  return std::make_tuple(wlen > 0, wlen);
}

std::tuple<bool, std::size_t> Socket::write_async(const char* buf, std::size_t len) {
  int wlen = ::send(fd_, buf, static_cast<int>(len), 0);
  if (wlen < 0) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(true, 0);
    return std::make_tuple(false, 0);
  }
  else if (wlen == 0) {
    return std::make_tuple(false, 0);
  }
  return std::make_tuple(true, wlen);
}

std::optional<SOCKET> create_server(std::uint16_t port) {
  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == INVALID_SOCKET)
    return {};

  sockaddr_in local_addr;
  local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(port);

  if (bind(sockfd, (const sockaddr*)&local_addr,
    static_cast<int>(sizeof(local_addr))) == SOCKET_ERROR)
    return {};
  if (listen(sockfd, 5) == SOCKET_ERROR)
    return {};

  return { sockfd };
}

void set_nonblocking(common::UniqueSocket& sockfd) {
  u_long flags = 1;
  ::ioctlsocket(sockfd, FIONBIO, &flags);
}

void serve_connection(common::UniqueSocket& sockfd) {
  if (send(sockfd, "*", 1, 0) < 1)
    return;

  NetStatus status = NetStatus::MSG_WAIT;
  for (;;) {
    char buf[1024];
    int rlen = recv(sockfd, buf, sizeof(buf), 0);
    if (rlen <= 0)
      break;

    for (int i = 0; i < rlen; ++i) {
      switch (status) {
      case NetStatus::MSG_WAIT:
        if (buf[i] == '^')
          status = NetStatus::MSG_RECV;
        break;
      case NetStatus::MSG_RECV:
        if (buf[i] == '$') {
          status = NetStatus::MSG_WAIT;
        }
        else {
          buf[i] += 1;
          if (send(sockfd, buf + i, 1, 0) < 0)
            return;
        }
        break;
      }
    }
  }
}

}