#include <WinSock2.h>
#include "examples.hh"

#pragma comment(lib, "ws2_32.lib")

class _WNGuarder {
public:
  _WNGuarder() noexcept {
    WSADATA wd;
    (void)WSAStartup(MAKEWORD(2, 2), &wd);
  }

  ~_WNGuarder() noexcept {
    WSACleanup();
  }
};

enum class _NetStatus {
  WAIT_MSG,
  IN_MSG,
};

static void serve_connection(SOCKET sockfd) {
  if (send(sockfd, "*", 1, 0) < 1) {
    shutdown(sockfd, SD_BOTH);
    closesocket(sockfd);
  }

  _NetStatus status = _NetStatus::WAIT_MSG;
  for (;;) {
    char buf[1024];
    int rlen = recv(sockfd, buf, sizeof(buf), 0);
    if (rlen <= 0)
      break;

    for (int i = 0; i < rlen; ++i) {
      char c = buf[i];
      switch (status) {
      case _NetStatus::WAIT_MSG:
        if (c == '^')
          status = _NetStatus::IN_MSG;
        break;
      case _NetStatus::IN_MSG:
        if (c == '$') {
          status = _NetStatus::WAIT_MSG;
        }
        else {
          c += 1;
          if (send(sockfd, &c, 1, 0) < 0) {
            shutdown(sockfd, SD_BOTH);
            closesocket(sockfd);
            return;
          }
        }
        break;
      }
    }
  }

  shutdown(sockfd, SD_BOTH);
  closesocket(sockfd);
}

EFW_EXAMPLE(SequentialServer, ss1) {
  _WNGuarder guard;

  SOCKET listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in local_addr;
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  local_addr.sin_port = htons(5555);

  bind(listen_sockfd, (const sockaddr*)&local_addr, sizeof(local_addr));
  listen(listen_sockfd, 5);

  for (;;) {
    sockaddr_in peer_addr;
    int peer_addr_len = static_cast<int>(sizeof(peer_addr));

    SOCKET new_sockfd = accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len);
    if (new_sockfd == INVALID_SOCKET)
      break;

    serve_connection(new_sockfd);
  }

  shutdown(listen_sockfd, SD_BOTH);
  closesocket(listen_sockfd);
}