#include "net/net.hh"
#include "msg_helper.hh"
#include "examples.hh"

namespace echo_server_mt {

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
      coro::async_wrap([](Socket conn) {
          coro::msg::on_blocking_serve(conn);
          conn.close();
        }, *c);
    }
  }
}

}

CORO_EXAMPLE(EchoServerThread, esmt, "an easy server with multi-thread") {
  echo_server_mt::launch();
}