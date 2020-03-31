#include "conet.hh"
#include "examples.hh"

namespace cs1 {

void launch_client() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> clt{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!clt->open() || !clt->connect("127.0.0.1"))
    return;

  char buf[1024];
  if (auto [r, n] = clt->read(buf, 1); !r || buf[0] != '*')
    return;

  coext::strv_t s = "^abc$defghi^jklmn$op^qrstuv$w^xyz$^0000$";
  if (auto [r, n] = clt->write(s.data(), s.size()); !r)
    return;
  std::cout << "CLIENT: send: " << s << std::endl;

  coext::str_t rbuf;
  for (;;) {
    auto [r, n] = clt->read(buf, sizeof(buf));
    if (!r)
      break;
    for (coext::sz_t i = 0; i < n; ++i)
      rbuf.push_back(buf[i]);

    if (rbuf.find("1111") != std::string::npos)
      break;
  }
  std::cout << "CLIENT: recv: " << rbuf << std::endl;

  std::cout << "CLIENT: disconnecting ..." << std::endl;
}

}

COEXT_EXAMPLE(SequentialClient, cc1, "A sequential receving client") {
  coext::WSGuarder guard;

  cs1::launch_client();
}