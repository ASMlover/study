#include "net/net.hh"
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server {

using coro::net::Socket;

void launch() {
  coro::net::Initializer<> init;

  std::unique_ptr<Socket, std::function<void (Socket*)>> server{
    new Socket{}, [](Socket* s) { s->close(); }
  };

  if (!server->open() || !server->listen())
    return;

  for (;;) {
    if (auto c = server->accept(); c) {
      coro::msg::on_blocking_serve(*c);
      (*c).close();
    }
  }
}

}

CORO_EXAMPLE(EchoServer, es, "an easy blocking server") {
  echo_server::launch();
}