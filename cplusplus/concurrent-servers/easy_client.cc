#include <iostream>
#include "common_utils.hh"
#include "examples.hh"

EFW_EXAMPLE(EasyClient, cc1) {
  common::WSGuarder guard;

  common::UniqueSocket sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  sockaddr_in remove_addr;
  remove_addr.sin_family = AF_INET;
  remove_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  remove_addr.sin_port = htons(5555);

  connect(sockfd, (const sockaddr*)&remove_addr, sizeof(remove_addr));

  char buf[1024];
  int rlen = recv(sockfd, buf, 1, 0);
  if (rlen <= 0 || buf[0] != '*')
    return;

  std::string_view s = "^abc$def^ghijk$lmn^000$";
  std::cout << "CLIENT: send: " << s << std::endl;
  send(sockfd, s.data(), static_cast<int>(s.size()), 0);

  std::string rbuf;
  for (;;) {
    std::memset(buf, 0, sizeof(buf));
    rlen = recv(sockfd, buf, sizeof(buf), 0);
    if (rlen <= 0)
      break;
    for (int i = 0; i < rlen; ++i)
      rbuf.push_back(buf[i]);

    if (rbuf.find("111") != std::string::npos)
      break;
  }
  std::cout << "CLIENT: recv: " << rbuf << std::endl;
}