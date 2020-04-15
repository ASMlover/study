#include "net/net.hh"
#include "msg_helper.hh"
#include "thread_pool.hh"
#include "examples.hh"

namespace echo_server_tp {

using coro::net::Socket;

void launch() {
  coro::net::Initializer<> init;
  coro::ThreadPool pool{4};

  std::unique_ptr<Socket, std::function<void (Socket*)>> server{
    new Socket{}, [](Socket* s) { s->close(); }
  };

  if (!server->open() || !server->listen())
    return;

  for (;;) {
    if (auto c = server->accept(); c) {
      pool.post_task([](Socket conn) {
          coro::msg::on_blocking_serve(conn);
          conn.close();
        }, *c);
    }
  }
}

}

CORO_EXAMPLE(EchoServerThreadPool,
  estp, "an easy blocking server with thread-pool") {
  echo_server_tp::launch();
}