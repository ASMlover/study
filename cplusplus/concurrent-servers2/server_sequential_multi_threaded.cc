#include "conet.hh"
#include "sequential_helper.hh"
#include "examples.hh"

namespace mss1 {

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

    coext::async_call([&conn] {
      sequential::serve_blocking_logic(conn);
      conn.close();
    });
  }
}

}

COEXT_EXAMPLE(SequentialServerMT, mss1, "A sequential server with multi-threaded") {
  coext::WSGuarder guard;

  mss1::launch_server();
}