#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

EFW_EXAMPLE(SequentialServer, ss1) {
  common::WSGuarder guard;

  common::UniqueSocket listen_sockfd;
  if (auto fd = svrutils::create_server(5555); !fd)
    return;
  else
    listen_sockfd.reset(*fd);

  for (;;) {
    sockaddr_in peer_addr;
    int peer_addr_len = static_cast<int>(sizeof(peer_addr));

    common::UniqueSocket new_sockfd(accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len));
    if (!new_sockfd)
      break;

    std::cout << "SERVER: accepct new connection: " << (SOCKET)new_sockfd << "|" << inet_ntoa(peer_addr.sin_addr) << std::endl;
    svrutils::serve_connection(new_sockfd);
    std::cout << "SERVER: connecttion: " << (SOCKET)new_sockfd << " disconnected" << std::endl;
  }
}