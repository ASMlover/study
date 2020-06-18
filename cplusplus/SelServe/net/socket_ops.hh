#pragma once

#include "types.hh"
#include "error.hh"

namespace sser::net::ops {

// socket state bits
enum class SockState : u8_t {
  NONE                      = 0x00,

  // the user wants a non-blocking socket
  USER_NON_BLOCKING         = 0x01,

  // the socket has been set non-blocking
  INTERNAL_NON_BLOCKING     = 0x02,

  // used to determine whether the socket is non-blocking
  NON_BLOCKING              = USER_NON_BLOCKING | INTERNAL_NON_BLOCKING,

  // user wants CONNECTION_ABORTED errors, which are disabled by default
  ENABLE_CONNECTION_ABORTED = 0x04,

  // user set the linger option, needs to be checked when closing
  USER_SET_LINGER           = 0x08,

  // the socket is stream-oriented
  STREAM_ORIENTED           = 0x10,

  // the socket is datagram-oriented
  DATAGRAM_ORIENTED         = 0x20,

  // the socket may have been dup()-ed
  POSSIBLE_DUP              = 0x40,
};

inline SockState operator|(SockState a, SockState b) noexcept {
  return as_type<SockState>(as_type<u8_t>(a) | as_type<u8_t>(b));
}

inline bool operator&(SockState a, SockState b) noexcept {
  return as_type<bool>(as_type<u8_t>(a) & as_type<u8_t>(b));
}

inline SockState operator~(SockState a) noexcept {
  return as_type<SockState>(~as_type<u8_t>(a));
}

inline SockState& operator|=(SockState& a, SockState b) noexcept {
  a = as_type<SockState>(as_type<u8_t>(a) | as_type<u8_t>(b));
  return a;
}

inline SockState& operator&=(SockState& a, SockState b) noexcept {
  a = as_type<SockState>(as_type<u8_t>(a) & as_type<u8_t>(b));
  return a;
}

socket_t open(int af, int type, int protocal, std::error_code& ec);
int shutdown(socket_t sockfd, int how, std::error_code& ec);
int close(socket_t sockfd, std::error_code& ec);

bool set_non_blocking(socket_t sockfd, bool non_blocking, std::error_code& ec);

int bind(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec);
int listen(socket_t sockfd, int backlog, std::error_code& ec);

socket_t accept(socket_t sockfd, void* addr, std::error_code& ec);
socket_t sync_accept(socket_t sockfd, SockState state, void* addr, std::error_code& ec);
bool non_blocking_accept(socket_t sockfd, SockState state, void* addr, std::error_code& ec, socket_t& new_sockfd);
int connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec);
void sync_connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec);
bool non_blocking_connect(socket_t sockfd, std::error_code& ec);

sz_t read(socket_t sockfd, void* buf, sz_t len, std::error_code& ec);
sz_t write(socket_t sockfd, const void* buf, sz_t len, std::error_code& ec);
sz_t read_from(socket_t sockfd, void* buf, sz_t len, void* addr, std::error_code& ec);
sz_t write_to(socket_t sockfd, const void* buf, sz_t len, strv_t host, u16_t port, std::error_code& ec);

int poll_connect(socket_t sockfd, int msec, std::error_code& ec);
int poll_read(socket_t sockfd, SockState state, int msec, std::error_code& ec);

int set_option(socket_t sockfd, SockState& state,
  int level, int optname, const void* optval, sz_t optlen, std::error_code& ec);
int get_option(socket_t sockfd, SockState state,
  int level, int optname, void* optval, sz_t* optlen, std::error_code& ec);

}