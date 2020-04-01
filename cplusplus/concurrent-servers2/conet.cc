#include "conet.hh"

namespace coext {

bool Socket::open() {
  if (is_valid())
    return true;

  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return is_valid();
}

void Socket::close() {
  if (is_valid()) {
    ::shutdown(fd_, SD_BOTH);
    ::closesocket(fd_);
    fd_ = INVALID_SOCKET;
  }
}

void Socket::set_nonblocking(bool mode) {
  if (is_valid()) {
    u_long flag = mode ? 1 : 0;
    ::ioctlsocket(fd_, FIONBIO, &flag);
  }
}

bool Socket::bind(strv_t host, u16_t port) {
  if (!is_valid())
    return false;

  sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(host.data());
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return ::bind(fd_, (const sockaddr*)&addr, static_cast<int>(sizeof(addr))) != SOCKET_ERROR;
}

bool Socket::listen(int backlog) {
  if (!is_valid())
    return false;

  return ::listen(fd_, backlog) != SOCKET_ERROR;
}

bool Socket::connect(strv_t host, u16_t port) {
  if (!is_valid())
    return false;

  sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(host.data());
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return ::connect(fd_, (const sockaddr*)&addr, static_cast<int>(sizeof(addr))) != SOCKET_ERROR;
}

std::tuple<bool, NetOption> Socket::connect_async(strv_t host, u16_t port) {
  if (!is_valid())
    return std::make_tuple(false, NetOption::OPTION_NONE);

  sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(host.data());
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (::connect(fd_, (const sockaddr*)&addr, static_cast<int>(sizeof(addr))) == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(false, NetOption::OPTION_AGAIN);
    return std::make_tuple(false, NetOption::OPTION_NONE);
  }
  return std::make_tuple(true, NetOption::OPTION_NONE);
}

std::tuple<bool, Socket> Socket::accept() {
  if (!is_valid())
    return std::make_tuple(false, Socket());

  sockaddr_in addr;
  int addr_len = static_cast<int>(sizeof(addr));
  Socket s = ::accept(fd_, (sockaddr*)&addr, &addr_len);

  return std::make_tuple(s.is_valid(), s);
}

std::tuple<bool, Socket, NetOption> Socket::accept_async() {
  if (!is_valid())
    return std::make_tuple(false, Socket(), NetOption::OPTION_NONE);

  sockaddr_in addr;
  int addr_len = static_cast<int>(sizeof(addr));
  Socket s = ::accept(fd_, (sockaddr*)&addr, &addr_len);

  if (!s.is_valid()) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(false, s, NetOption::OPTION_AGAIN);
    return std::tuple(false, s, NetOption::OPTION_NONE);
  }
  return std::make_tuple(true, s, NetOption::OPTION_NONE);
}

std::tuple<bool, sz_t> Socket::read(char* buf, sz_t len) {
  if (!is_valid())
    return std::make_tuple(false, 0);

  int r = ::recv(fd_, buf, static_cast<int>(len), 0);
  return std::make_tuple(r > 0, r);
}

std::tuple<bool, sz_t, NetOption> Socket::read_async(char* buf, sz_t len) {
  if (!is_valid())
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);

  int r = ::recv(fd_, buf, static_cast<int>(len), 0);
  if (r == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(false, 0, NetOption::OPTION_AGAIN);
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);
  }
  else if (r == 0) {
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);
  }
  return std::make_tuple(true, r, NetOption::OPTION_NONE);
}

std::tuple<bool, sz_t> Socket::write(const char* buf, sz_t len) {
  if (!is_valid())
    return std::make_tuple(false, 0);

  int r = ::send(fd_, buf, static_cast<int>(len), 0);
  return std::make_tuple(r > 0, r);
}

std::tuple<bool, sz_t, NetOption> Socket::write_async(const char* buf, sz_t len) {
  if (!is_valid())
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);

  int r = ::send(fd_, buf, static_cast<int>(len), 0);
  if (r == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK)
      return std::make_tuple(false, 0, NetOption::OPTION_AGAIN);
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);
  }
  else if (r == 0) {
    return std::make_tuple(false, 0, NetOption::OPTION_NONE);
  }
  return std::make_tuple(true, r, NetOption::OPTION_NONE);
}

}