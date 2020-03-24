#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

namespace mts { // multi-threaded server

void launch_server() {
  common::UniqueSocket listen_sockfd;
  if (auto fd = svrutils::create_server(5555); !fd)
    return;
  else
    listen_sockfd.reset(*fd);

  for (;;) {
    sockaddr_in peer_addr;
    int peer_addr_len = static_cast<int>(sizeof(peer_addr));

    SOCKET new_sockfd = accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len);
    if (new_sockfd == INVALID_SOCKET)
      break;

    auto _ = common::async([](SOCKET fd) {
        common::UniqueSocket sockfd(fd);
        svrutils::serve_connection(sockfd);
      }, new_sockfd);
    (void)_;
  }
}

}

EFW_EXAMPLE(MultiThreadServer, ms1) {
  common::WSGuarder guard;

  mts::launch_server();
}