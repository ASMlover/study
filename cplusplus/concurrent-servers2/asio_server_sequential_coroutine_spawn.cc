#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "sequential_helper.hh"
#include "examples.hh"

namespace acospawn {
// boost.coroutine spawn deprecated

using boost::asio::ip::tcp;

void on_serve(boost::asio::io_context& io_context, tcp::socket s) {
  boost::asio::strand<boost::asio::io_context::executor_type> strand(io_context.get_executor());

  boost::asio::spawn(strand, [&](boost::asio::yield_context yield) {
    try {
      boost::system::error_code ec;
      boost::asio::async_write(s, boost::asio::buffer("*"), yield[ec]);

      sequential::Status status = sequential::Status::WAIT_MSG;
      for (;;) {
        char recv_buf[1024];
        auto n = s.async_read_some(boost::asio::buffer(recv_buf), yield[ec]);
        if (ec)
          break;

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

        boost::asio::async_write(s, boost::asio::buffer(send_buf), yield[ec]);
        if (ec)
          break;
      }
      s.close();
    }
    catch (const std::exception& /*e*/) {
      s.close();
    }
  });
}

void launch_server() {
  boost::asio::io_context io_context;

  boost::asio::spawn(io_context,
    [&](boost::asio::yield_context yield) {
      tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5555));

      for (;;) {
        boost::system::error_code ec;
        tcp::socket s(io_context);
        acceptor.async_accept(s, yield[ec]);
        if (!ec)
          on_serve(io_context, std::move(s));
      }
    });
}

}

COEXT_EXAMPLE(AsioSequentialServerCoroutineSpawn,
  acospawn, "A sequential server with boost.asio coroutine spawn") {
  acospawn::launch_server();
}