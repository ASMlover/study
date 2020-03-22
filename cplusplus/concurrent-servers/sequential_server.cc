#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

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
    svrutils::serve_connection(new_sockfd);
    std::cout << "SERVER: connecttion: " << (SOCKET)new_sockfd << " disconnected" << std::endl;
  }
}