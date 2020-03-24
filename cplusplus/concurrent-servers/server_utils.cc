#include "server_utils.hh"

namespace svrutils {

enum class NetStatus {
  MSG_WAIT,
  MSG_RECV,
};

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