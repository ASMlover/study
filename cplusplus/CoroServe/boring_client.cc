#include "net/net.hh"
#include "examples.hh"

namespace boring_client {

using coro::net::Socket;

void launch() {
  coro::net::Initializer<> init;

  std::unique_ptr <Socket, std::function<void (Socket*)>> c{
    new Socket{}, [](Socket* s) { s->close(); }
  };

  if (!c->start_connect())
    return;

  for (;;) {
    char rbuf[1024]{};
    if (auto n = c->read(rbuf, sizeof(rbuf)); n > 0)
      std::cout << "BORING CLIENT: recv: " << rbuf << std::endl;
    else
      break;
  }
}

}

CORO_EXAMPLE(BoringClient, bc, "a boring client") {
  boring_client::launch();
}