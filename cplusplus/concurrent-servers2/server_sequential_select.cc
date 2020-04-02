#include "conet.hh"
#include "sequential_helper.hh"
#include "examples.hh"

namespace sss1 {

static constexpr int EVNO = 0; // non-event
static constexpr int EVRD = 1; // read-event
static constexpr int EVWR = 2; // write-event

class Conn final : private coext::UnCopyable {
  coext::Socket s_;
  sequential::Status status_{ sequential::Status::INIT_ACK };
  std::vector<char> send_buf_;
public:
  Conn(coext::Socket s) noexcept : s_(std::move(s)) {
    s_.set_nonblocking();
  }

  ~Conn() noexcept {
    s_.close();
  }

  inline const coext::Socket& get_socket() const noexcept { return s_; }
  inline coext::Socket& get_socket() noexcept { return s_; }
  inline coext::socket_t get_sockfd() const noexcept { return s_.get(); }
  inline void close() { s_.close(); }
  inline bool is_valid() const noexcept { return s_.is_valid(); }

  std::optional<coext::Socket> accept() {
    if (auto [r, s, opt] = s_.accept_async();
      r || (!r && opt == coext::NetOption::OPTION_AGAIN))
      return { s };
    return {};
  }

  bool on_event_connected() {
    status_ = sequential::Status::INIT_ACK;
    send_buf_.clear();
    send_buf_.push_back('*');

    return true;
  }

  bool on_event_read() {
    if (status_ == sequential::Status::INIT_ACK || !send_buf_.empty())
      return true;

    char buf[1024];
    if (auto [r, n, opt] = s_.read_async(buf, sizeof(buf));
      r || (!r && opt == coext::NetOption::OPTION_AGAIN)) {
      for (int i = 0; i < n; ++i) {
        switch (status_) {
        case sequential::Status::INIT_ACK: break;
        case sequential::Status::WAIT_MSG:
          if (buf[i] == '^')
            status_ = sequential::Status::READ_MSG;
          break;
        case sequential::Status::READ_MSG:
          if (buf[i] == '$')
            status_ = sequential::Status::WAIT_MSG;
          else
            send_buf_.push_back(buf[i] + 1);
          break;
        }
      }
      return true;
    }

    close();
    return false;
  }

  bool on_event_write() {
    if (send_buf_.empty())
      return true;

    if (auto [r, n, opt] = s_.write_async(send_buf_.data(), send_buf_.size());
      r || (!r && opt == coext::NetOption::OPTION_AGAIN)) {
      if (n <= 0)
        return true;

      if (n < send_buf_.size()) {
        send_buf_.erase(send_buf_.begin(), send_buf_.begin() + n);
      }
      else {
        send_buf_.clear();
        if (status_ == sequential::Status::INIT_ACK)
          status_ = sequential::Status::WAIT_MSG;
      }
      return true;
    }

    close();
    return false;
  }
};
using ConnPtr = std::shared_ptr<Conn>;

class EventLoop final : private coext::UnCopyable {
  coext::Socket acceptor_{};
  std::list<ConnPtr> conns_;
  fd_set rfds_master_;
  fd_set wfds_master_;

  inline bool is_acceptor(coext::socket_t fd) const { return acceptor_.get() == fd; }

  void add_event(coext::socket_t sockfd, int ev) {
    if (ev & EVRD)
      FD_SET(sockfd, &rfds_master_);
    if (ev & EVWR)
      FD_SET(sockfd, &wfds_master_);
  }

  void del_event(coext::socket_t sockfd, int ev) {
    if (ev & EVRD)
      FD_CLR(sockfd, &rfds_master_);
    if (ev & EVWR)
      FD_CLR(sockfd, &wfds_master_);
  }

  ConnPtr append_conn(coext::Socket s, int ev) {
    auto conn = std::make_shared<Conn>(s);
    conns_.push_back(conn);
    add_event(conn->get_sockfd(), ev);

    return conn;
  }
public:
  EventLoop() noexcept {
    FD_ZERO(&rfds_master_);
    FD_ZERO(&wfds_master_);
  }

  ~EventLoop() noexcept {
    conns_.clear();
    acceptor_.close();
  }

  void poll() {
    fd_set rfds = rfds_master_;
    fd_set wfds = wfds_master_;
    int nready = ::select(-1, &rfds, &wfds, nullptr, nullptr);
    if (nready < 0)
      return;

    for (auto it = conns_.begin(); it != conns_.end();) {
      if (nready <= 0)
        break;

      auto& conn = *it;
      auto sockfd = conn->get_sockfd();
      bool been_slove = false;
      if (FD_ISSET(sockfd, &rfds)) {
        been_slove = true;
        if (is_acceptor(sockfd)) {
          if (auto s = conn->accept(); s) {
            auto new_conn = append_conn(*s, EVRD | EVWR);
            new_conn->on_event_connected();
          }
          else {
            return;
          }
        }
        else {
          if (!conn->on_event_read())
            del_event(sockfd, EVRD | EVWR);
        }
      }
      if (FD_ISSET(sockfd, &wfds) && conn->is_valid()) {
        been_slove = true;
        if (!conn->on_event_write())
          del_event(sockfd, EVRD | EVWR);
      }

      if (been_slove)
        --nready;

      if (!conn->is_valid())
        conns_.erase(it++);
      else
        ++it;
    }
  }

  bool start_listen(coext::strv_t host = "0.0.0.0", coext::u16_t port = 5555) {
    if (!acceptor_.open() || !acceptor_.bind(host, port) || !acceptor_.listen())
      return false;
    append_conn(acceptor_, EVRD);
    return true;
  }

  void run() {
    for (;;)
      poll();
  }
};

void launch_server() {
  EventLoop evloop{};

  evloop.start_listen();
  evloop.run();
}

}

COEXT_EXAMPLE(SequentialServerSelect, sss1, "A sequential server with select") {
  coext::WSGuarder guard;

  sss1::launch_server();
}