#include "net/net.hh"
#include "examples.hh"

namespace boring_server_block {

using coro::net::Socket;

void on_boring(Socket c, coro::strv_t msg) {
  for (int i = 0; i < 5; ++i) {
    if (auto n = c.write(msg.data(), msg.size()); n <= 0)
      return;
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 500 + 1));
  }

  coro::strv_t quit = "I'm boring, I'm quit!";
  c.write(quit.data(), quit.size());
}

void launch() {
  coro::net::Initializer<> init;

  std::unique_ptr<Socket, std::function<void (Socket*)>> server{
    new Socket{}, [](Socket* s) { s->close(); }
  };

  if (!server->start_listen())
    return;

  for (;;) {
    if (auto s = server->accept(); s) {
      on_boring(*s, "boring!");
      (*s).close();
    }
    else {
      break;
    }
  }
}

}

CORO_EXAMPLE(BoringServerBlocking, bsb, "a blocking boring server") {
  boring_server_block::launch();
}