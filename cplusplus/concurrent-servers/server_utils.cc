#include "server_utils.hh"

namespace svrutils {

enum class NetStatus {
  MSG_WAIT,
  MSG_RECV,
};

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