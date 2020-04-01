#include "conet.hh"
#include "sequential_helper.hh"
#include "examples.hh"

namespace ss1 {

void launch_server() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;

  for (;;) {
    auto [r, conn] = svr->accept();
    if (!r)
      break;

    std::cout << "SERVER: accept new connection " << conn.get() << " at " << time(nullptr) << std::endl;
    sequential::serve_blocking_logic(conn);
    std::cout << "SERVRE: connection " << conn.get() << " disconnected" << std::endl;

    conn.close();
  }
}

}

COEXT_EXAMPLE(SequentialServerBlock, ss1, "A sequential blocking server") {
  coext::WSGuarder guard;

  ss1::launch_server();
}