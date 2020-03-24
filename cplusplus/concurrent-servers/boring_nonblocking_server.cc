#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

namespace nbs {

void launch_server() {
  common::UniqueSocket listen_sockfd;
  if (auto fd = svrutils::create_server(5555); !fd)
    return;
  else
    listen_sockfd.reset(*fd);

  sockaddr_in peer_addr;
  int peer_addr_len = static_cast<int>(sizeof(peer_addr));
  common::UniqueSocket new_sockfd(accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len));
  if (!new_sockfd)
    return;
  if (send(new_sockfd, "*", 1, 0) < 1)
    return;

  svrutils::set_nonblocking(new_sockfd);
  for (;;) {
    char buf[1024];
    std::cout << "calling recv ..." << std::endl;
    int len = recv(new_sockfd, buf, sizeof(buf), 0);
    if (len < 0) {
      auto err = WSAGetLastError();
      if (err == WSAEWOULDBLOCK) {
        Sleep(10);
        continue;
      }
      break;
    }
    else if (len == 0) {
      std::cout << "peer disconnected, i'm done !" << std::endl;
      break;
    }
    std::cout << "recv returned " << len << " bytes" << std::endl;
  }
}

}

EFW_EXAMPLE(BoringNBServer, ss2) {
  common::WSGuarder guard;

  nbs::launch_server();
}