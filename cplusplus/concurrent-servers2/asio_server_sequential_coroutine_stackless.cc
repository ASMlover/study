#include <boost/asio.hpp>
#include <boost/asio/yield.hpp>
#include "sequential_helper.hh"
#include "examples.hh"

namespace acosl1 {
// asio stackless coroutine sequential server

using boost::asio::ip::tcp;

class TcpServer final
  : private coext::UnCopyable, public boost::asio::coroutine {
  tcp::acceptor acceptor_;
  tcp::socket s_;
  sequential::Status status_{ sequential::Status::INIT_ACK };
  std::vector<char> recv_buf_;
  std::vector<char> send_buf_;

  struct ref {
    TcpServer* s_;

    ref(TcpServer* s) noexcept : s_{s} {}
    void operator()(std::error_code ec, coext::sz_t n = 0) { (*s_)(ec, n); }
  };
public:
  TcpServer(boost::asio::io_context& io_context, coext::u16_t port) noexcept
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , s_(io_context) {
    recv_buf_.resize(1024);
  }

  void operator()(std::error_code ec, coext::sz_t n = 0) {
    reenter(this) for (;;) {
      yield acceptor_.async_accept(s_, ref(this));

      if (!ec) {
        yield boost::asio::async_write(s_, boost::asio::buffer("*", 1), ref(this));

        status_ = sequential::Status::WAIT_MSG;
        while (!ec) {
          yield s_.async_read_some(boost::asio::buffer(recv_buf_), ref(this));

          if (!ec) {
            send_buf_.clear();
            for (int i = 0; i < n; ++i) {
              switch (status_) {
              case sequential::Status::INIT_ACK: break;
              case sequential::Status::WAIT_MSG:
                if (recv_buf_[i] == '^')
                  status_ = sequential::Status::READ_MSG;
                break;
              case sequential::Status::READ_MSG:
                if (recv_buf_[i] == '$')
                  status_ = sequential::Status::WAIT_MSG;
                else
                  send_buf_.push_back(recv_buf_[i] + 1);
                break;
              }
            }
            yield boost::asio::async_write(s_, boost::asio::buffer(send_buf_), ref(this));
          }
        }
      }

      s_.close();
    }
  }
};

void launch_server() {
  boost::asio::io_context io_context;
  auto server = std::make_shared<TcpServer>(io_context, 5555);
  (*server)(std::error_code());

  io_context.run();
}

}

COEXT_EXAMPLE(AsioSequentialServerCoroutineStackless, acosl1,
  "A sequential server with boost.asio stackless coroutine") {
  acosl1::launch_server();
}