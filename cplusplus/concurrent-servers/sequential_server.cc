#include <iostream>
#include "common_utils.hh"
#include "examples.hh"

enum class _NetStatus {
  WAIT_MSG,
  IN_MSG,
};

static void serve_connection(common::UniqueSocket& sockfd) {
  if (send(sockfd, "*", 1, 0) < 1)
    return;

  _NetStatus status = _NetStatus::WAIT_MSG;
  for (;;) {
    char buf[1024];
    int rlen = recv(sockfd, buf, sizeof(buf), 0);
    if (rlen <= 0)
      break;

    for (int i = 0; i < rlen; ++i) {
      switch (status) {
      case _NetStatus::WAIT_MSG:
        if (buf[i] == '^')
          status = _NetStatus::IN_MSG;
        break;
      case _NetStatus::IN_MSG:
        if (buf[i] == '$') {
          status = _NetStatus::WAIT_MSG;
        }
        else {
          buf[i] += 1;
          if (send(sockfd, buf+i, 1, 0) < 0)
            return;
        }
        break;
      }
    }
  }
}

EFW_EXAMPLE(SequentialServer, ss1) {
  common::WSGuarder guard;

  common::UniqueSocket listen_sockfd(socket(AF_INET, SOCK_STREAM, 0));
  if (!listen_sockfd)
    return;

  sockaddr_in local_addr;
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  local_addr.sin_port = htons(5555);

  bind(listen_sockfd, (const sockaddr*)&local_addr, sizeof(local_addr));
  listen(listen_sockfd, 5);

  for (;;) {
    sockaddr_in peer_addr;
    int peer_addr_len = static_cast<int>(sizeof(peer_addr));

    common::UniqueSocket new_sockfd(accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len));
    if (!new_sockfd)
      break;

    std::cout << "SERVER: accepct new connection: " << (SOCKET)new_sockfd << "|" << inet_ntoa(peer_addr.sin_addr) << std::endl;
    serve_connection(new_sockfd);
    std::cout << "SERVER: connecttion: " << (SOCKET)new_sockfd << " disconnected" << std::endl;
  }
}