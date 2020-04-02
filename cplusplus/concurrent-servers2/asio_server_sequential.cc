#include <boost/asio.hpp>
#include "sequential_helper.hh"
#include "examples.hh"

namespace ass1 {

using boost::asio::ip::tcp;

class TcpSession final
  : private coext::UnCopyable
  , public std::enable_shared_from_this<TcpSession> {
  tcp::socket s_;
  sequential::Status status_{ sequential::Status::INIT_ACK };
  std::vector<char> read_buf_;
  std::vector<char> send_buf_;

  void on_read_impl (coext::sz_t len) {
    for (coext::sz_t i = 0; i < len; ++i) {
      switch (status_) {
      case sequential::Status::INIT_ACK: break;
      case sequential::Status::WAIT_MSG:
        if (read_buf_[i] == '^')
          status_ = sequential::Status::READ_MSG;
        break;
      case sequential::Status::READ_MSG:
        if (read_buf_[i] == '$')
          status_ = sequential::Status::WAIT_MSG;
        else
          send_buf_.push_back(read_buf_[i] + 1);
        break;
      }
    }
  }

  void do_ack() {
    auto self(shared_from_this());
    boost::asio::async_write(s_, boost::asio::buffer(send_buf_),
      [this, self](const std::error_code& ec, coext::sz_t /*len*/) {
        if (!ec) {
          status_ = sequential::Status::WAIT_MSG;
          send_buf_.clear();
          do_read();
        }
      });
  }

  void do_read() {
    if (status_ == sequential::Status::INIT_ACK) {
      do_ack();
      return;
    }

    auto self(shared_from_this());
    s_.async_read_some(boost::asio::buffer(read_buf_),
      [this, self](const std::error_code& ec, coext::sz_t len) {
        if (!ec) {
          on_read_impl(len);
          do_write();
        }
      });
  }

  void do_write() {
    auto self(shared_from_this());
    boost::asio::async_write(s_, boost::asio::buffer(send_buf_),
      [this, self](const std::error_code& ec, coext::sz_t /*len*/) {
        if (!ec) {
          send_buf_.clear();
          do_read();
        }
      });
  }
public:
  TcpSession(tcp::socket&& s) noexcept : s_(std::move(s)) {
    read_buf_.resize(1024);
  }

  ~TcpSession() noexcept {
    std::cout << "TcpSession<" << this << "> disconnected" << std::endl;
  }

  void start() {
    status_ = sequential::Status::INIT_ACK;
    send_buf_.clear();
    send_buf_.push_back('*');

    do_ack();
  }
};

class TcpServer final : private coext::UnCopyable {
  tcp::acceptor acceptor_;
  tcp::socket s_;

  void do_accept() {
    acceptor_.async_accept(s_,
      [this](const std::error_code& ec) {
        if (!ec)
          std::make_shared<TcpSession>(std::move(s_))->start();

        do_accept();
      });
  }
public:
  TcpServer(boost::asio::io_context& io_context, coext::u16_t port) noexcept
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    , s_(io_context) {
  }

  void start() {
    do_accept();
  }
};

void launch_server() {
  boost::asio::io_context io_context;
  TcpServer s{ io_context, 5555 };
  s.start();

  io_context.run();
}

}

COEXT_EXAMPLE(AsioSequentialServer, ass1, "A sequential server with boost.asio") {
  ass1::launch_server();
}