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
  int ev_{ EVNO };
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
  inline void close() { s_.close(); }

  inline bool is_valid() const noexcept { return s_.is_valid(); }
  inline bool is_nev() const noexcept { return ev_ == EVNO; }
  inline bool is_rev() const noexcept { return ev_ | EVRD; }
  inline bool is_wev() const noexcept { return ev_ | EVWR; }
  inline int get_ev() const noexcept { return ev_; }
  inline void set_ev(int ev) noexcept { ev_ |= ev; }
  inline void clr_ev(int ev) noexcept { ev_ &= ~ev; }

  std::optional<coext::Socket> accept() {
    if (auto [r, s, opt] = s_.accept_async();
      r || (!r && opt == coext::NetOption::OPTION_AGAIN))
      return { s };
    return {};
  }

  void on_event_connected() {
    status_ = sequential::Status::INIT_ACK;
    send_buf_.clear();
    send_buf_.push_back('*');
  }

  void on_event_read() {
    if (status_ == sequential::Status::INIT_ACK || !send_buf_.empty()) {
      ev_ |= EVWR;
      return;
    }

    char buf[1024];
    if (auto [r, n, opt] = s_.read_async(buf, sizeof(buf));
      r || (!r && opt == coext::NetOption::OPTION_AGAIN)) {
      ev_ |= EVRD;
      for (int i = 0; i < n; ++i) {
        switch (status_) {
        case sequential::Status::INIT_ACK: break;
        case sequential::Status::WAIT_MSG:
          if (buf[i] == '^')
            status_ = sequential::Status::READ_MSG;
          break;
        case sequential::Status::READ_MSG:
          if (buf[i] == '$') {
            status_ = sequential::Status::WAIT_MSG;
          }
          else {
            send_buf_.push_back(buf[i] + 1);
            ev_ = EVWR;
          }
          break;
        }
      }
    }
    else {
      ev_ = EVNO;
    }
  }

  void on_event_write() {
    if (send_buf_.empty()) {
      ev_ |= (EVRD | EVWR);
      return;
    }

    if (auto [r, n, opt] = s_.write_async(send_buf_.data(), send_buf_.size());
      r || (!r && opt == coext::NetOption::OPTION_AGAIN)) {
      ev_ |= EVWR;

      if (n <= 0)
        return;

      if (n < send_buf_.size()) {
        send_buf_.erase(send_buf_.begin(), send_buf_.begin() + n);
        ev_ |= EVWR;
      }
      else {
        send_buf_.clear();
        if (status_ == sequential::Status::INIT_ACK)
          status_ = sequential::Status::WAIT_MSG;
        ev_ |= EVRD;
      }
    }
    else {
      ev_ = EVNO;
    }
  }
};
using ConnPtr = std::shared_ptr<Conn>;

class EventLoop final : private coext::UnCopyable {
  std::list<ConnPtr> conns_;

  coext::socket_t listenfd_{};
  fd_set rfds_master_;
  fd_set wfds_master_;
public:
  EventLoop(coext::Socket listen_sock) noexcept
    : listenfd_(listen_sock.get()) {
    FD_ZERO(&rfds_master_);
    FD_ZERO(&wfds_master_);

    append_conn(listen_sock, EVRD);
  }

  void update_event(ConnPtr& conn) {
    auto fd = conn->get_socket().get();
    int ev = conn->get_ev();
    if (ev & EVRD)
      FD_SET(fd, &rfds_master_);
    else
      FD_CLR(fd, &rfds_master_);
    if (ev & EVWR)
      FD_SET(fd, &wfds_master_);
    else
      FD_CLR(fd, &wfds_master_);
  }

  ConnPtr append_conn(coext::Socket s, int ev) {
    auto conn = std::make_shared<Conn>(s);
    conns_.push_back(conn);

    conn->set_ev(ev);
    update_event(conn);
    return conn;
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
      auto sockfd = conn->get_socket().get();

      bool been_slove = false;
      if (FD_ISSET(sockfd, &rfds)) {
        been_slove = true;
        if (sockfd == listenfd_) {
          if (auto s = conn->accept(); s) {
            auto new_conn = append_conn(*s, EVRD | EVWR);
            new_conn->on_event_connected();
          }
          else {
            return;
          }
        }
        else {
          conn->on_event_read();
          update_event(conn);

          if (conn->is_nev())
            conn->close();
        }
      }
      if (FD_ISSET(sockfd, &wfds) && conn->is_valid()) {
        been_slove = true;
        conn->on_event_write();
        update_event(conn);

        if (conn->is_nev())
          conn->close();
      }

      if (been_slove)
        --nready;

      if (!conn->is_valid())
        conns_.erase(it++);
      else
        ++it;
    }
  }

  void run_poll() {
    for (;;)
      poll();
  }
};

void launch_server() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;

  EventLoop{ *svr }.run_poll();
}

}

COEXT_EXAMPLE(SequentialServerSelect, sss1, "A sequential server with select") {
  coext::WSGuarder guard;

  sss1::launch_server();
}