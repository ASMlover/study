#include <sstream>
#include "common_utils.hh"
#include "examples.hh"

EFW_EXAMPLE(BoringClient, bc1) {
  common::WSGuarder guard;

  common::UniqueSocket sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  sockaddr_in remove_addr;
  remove_addr.sin_family = AF_INET;
  remove_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  remove_addr.sin_port = htons(5555);

  if (connect(sockfd,
    (const sockaddr*)&remove_addr, sizeof(remove_addr)) == SOCKET_ERROR)
    return;
  std::cout << "CLIENT: connected ..." << std::endl;

  for (int i = 0; i < 5; ++i) {
    std::stringstream ss;
    ss << i << i << i << i << i;
    std::string s("boring string -> ");
    s += ss.str();

    send(sockfd, s.data(), static_cast<int>(s.size()), 0);
    std::cout << "CLIENT: send: " << s << std::endl;
  }

  std::cout << "CLIENT: disconnecting ..." << std::endl;
}