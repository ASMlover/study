#include "conet.hh"
#include "sequential_helper.hh"
#include "examples.hh"

namespace tps1 {

void launch_server() {
  coext::ThreadPool pool(4);
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;

  for (;;) {
    auto [r, conn] = svr->accept();
    if (!r)
      break;

    pool.run_task([&conn] {
      std::cout << "SERVER: current thread: " << std::this_thread::get_id() << std::endl;
      sequential::serve_blocking_logic(conn);
      conn.close();
    });
  }
}

}

COEXT_EXAMPLE(SequentialServerThreadPool, tps1, "A sequential server with thread-pool") {
  coext::WSGuarder guard;

  tps1::launch_server();
}