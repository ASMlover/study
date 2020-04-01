#include "conet.hh"
#include "examples.hh"

namespace bs1 {

void launch_server() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket(), [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;

  auto [r, conn] = svr->accept();
  if (!r)
    return;

  for (;;) {
    char buf[1024]{};
    if (auto [r, n] = conn.read(buf, sizeof(buf)); r) {
      std::cout << "SERVER: receive " << n << " bytes from client: " << buf << std::endl;
    }
    else {
      std::cout << "SERVER: client disconnected ..." << std::endl;
      break;
    }
  }
  conn.close();
}

}

COEXT_EXAMPLE(BoringServerBlock, bs1, "A blocking boring receiving server") {
  coext::WSGuarder guard;

  bs1::launch_server();
}