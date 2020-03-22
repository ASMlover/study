#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

namespace mts { // multi-threaded server

static void server_thread(SOCKET fd) {
  common::UniqueSocket sockfd(fd);

  svrutils::serve_connection(sockfd);
}

void launch_server() {
  common::UniqueSocket listen_sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  if (!listen_sockfd)
    return;

  sockaddr_in local_addr;
  local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(5555);

  bind(listen_sockfd, (const sockaddr*)&local_addr, static_cast<int>(sizeof(local_addr)));
  listen(listen_sockfd, 5);

  for (;;) {
    sockaddr_in peer_addr;
    int peer_addr_len = static_cast<int>(sizeof(peer_addr));

    SOCKET new_sockfd = accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len);
    if (new_sockfd == INVALID_SOCKET)
      break;

    common::async(server_thread, new_sockfd);
  }
}

}

EFW_EXAMPLE(MultiThreadServer, ms1) {
  common::WSGuarder guard;

  mts::launch_server();
}