#include <boost/asio.hpp>
#include "sequential_helper.hh"
#include "examples.hh"

namespace acosf1 {
// asio stackful coroutine sequential server

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

awaitable<void> on_serve(tcp::socket s) {
  try {
    co_await boost::asio::async_write(s, boost::asio::buffer("*", 1), use_awaitable);

    char recv_buf[1024];
    auto status = sequential::Status::WAIT_MSG;
    for (;;) {
      coext::sz_t n = co_await s.async_read_some(boost::asio::buffer(recv_buf), use_awaitable);

      std::vector<char> send_buf;
      for (coext::sz_t i = 0; i < n; ++i) {
        switch (status) {
        case sequential::Status::INIT_ACK: break;
        case sequential::Status::WAIT_MSG:
          if (recv_buf[i] == '^')
            status = sequential::Status::READ_MSG;
          break;
        case sequential::Status::READ_MSG:
          if (recv_buf[i] == '$')
            status = sequential::Status::WAIT_MSG;
          else
            send_buf.push_back(recv_buf[i] + 1);
          break;
        }
      }
      co_await boost::asio::async_write(s, boost::asio::buffer(send_buf), use_awaitable);
    }
  }
  catch (const std::exception& /*e*/) {
    s.close();
  }
}

awaitable<void> on_accept() {
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, { tcp::v4(), 5555 });
  for (;;) {
    tcp::socket s = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, [s = std::move(s)]() mutable {
      return on_serve(std::move(s));
    }, detached);
  }
}

void launch_server() {
  try {
    boost::asio::io_context io_context;

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context](auto, auto) { io_context.stop(); });

    co_spawn(io_context, on_accept, detached);
    io_context.run();
  }
  catch (const std::exception& e) {
    std::cerr << "launch_server exception: " << e.what() << std::endl;
  }
}

}

COEXT_EXAMPLE(AsioSequentialServerCoroutineStackful, acosf1,
  "A sequential server with boost.asio stackful coroutine") {
  acosf1::launch_server();
}
