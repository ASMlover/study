#include "conet.hh"
#include "examples.hh"

namespace bs2 {

void launch_server() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;

  auto [r, conn] = svr->accept();
  if (!r)
    return;

  conn.set_nonblocking();
  for (;;) {
    char buf[1024]{};
    if (auto [r, n, opt] = conn.read_async(buf, sizeof(buf)); !r) {
      if (opt == coext::NetOption::OPTION_AGAIN) {
        ::Sleep(1);
        continue;
      }
      break;
    }
    else {
      std::cout << "SERVER: receive " << n << " bytes from client: " << buf << std::endl;
    }
  }
  std::cout << "SERVER: client disconnected ..." << std::endl;
  conn.close();
}

}

COEXT_EXAMPLE(BoringServerNonBlock, bs2, "A nonblocking boring receiving server") {
  coext::WSGuarder guard;

  bs2::launch_server();
}