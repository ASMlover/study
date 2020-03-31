#include "conet.hh"
#include "examples.hh"

namespace bc1 {

void launch_client() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> clt{
    new coext::Socket(), [](coext::Socket* s) { s->close(); }
  };

  clt->open();
  if (!clt->connect("127.0.0.1"))
    return;

  std::cout << "CLIENT: connect to server success" << std::endl;
  for (int i = 0; i < 10; ++i) {
    coext::ss_t ss;
    ss << "[" << i << "] boring message";
    coext::str_t s(ss.str());

    auto [r, n] = clt->write(s.data(), s.size());
    if (r)
      std::cout << "CLIENT: send message to server: " << s << std::endl;
    else
      break;
    ::Sleep(1);
  }
}

}

COEXT_EXAMPLE(BoringClient, bc1, "A boring sending client") {
  coext::WSGuarder guard;

  bc1::launch_client();
}