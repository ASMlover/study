#include <unordered_map>
#include <list>
#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

namespace ss {

static constexpr int EVENT_NO = 0; // no event
static constexpr int EVENT_RD = 1; // read event
static constexpr int EVENT_WR = 2; // write event

class Conn : private common::UnCopyable {
  enum class State {
    INIT_ACK,
    WAIT_MSG,
    READ_MSG,
  };

  svrutils::Socket s_;
  State state_{ State::INIT_ACK };
  int event_{ EVENT_NO };
  std::vector<char> send_buff_;
public:
  Conn(SOCKET fd) noexcept
    : s_(fd) {
    s_.set_nonblocking();
  }

  Conn(svrutils::Socket s) noexcept
    : s_(s) {
    s_.set_nonblocking();
  }

  inline svrutils::Socket& get_socket() noexcept { return s_; }

  inline bool is_nevent() const noexcept { return event_ == EVENT_NO; }
  inline bool is_revent() const noexcept { return event_ | EVENT_RD; }
  inline bool is_wevent() const noexcept { return event_ | EVENT_WR; }
  inline void set_event(int ev) noexcept { event_ |= ev; }
  inline void cls_event(int ev) noexcept { event_ &= ~ev; }
  inline int get_event() const noexcept { return event_; }

  void on_handle_connected() {
    state_ = State::INIT_ACK;
    send_buff_.clear();
    send_buff_.push_back('*');
    event_ = EVENT_RD;
  }

  void on_handle_read() {
    if (state_ == State::INIT_ACK || !send_buff_.empty())
      event_ |= EVENT_WR;

    char buf[1024];
    auto [r, bytes] = s_.read_async(buf, sizeof(buf));
    if (!r) {
      event_ = EVENT_NO;
      return;
    }
    else {
      event_ |= EVENT_RD;
    }

    for (int i = 0; i < bytes; ++i) {
      switch (state_) {
      case State::INIT_ACK: break;
      case State::WAIT_MSG:
        if (buf[i] == '^')
          state_ = State::READ_MSG;
        break;
      case State::READ_MSG:
        if (buf[i] == '$') {
          state_ = State::WAIT_MSG;
        }
        else {
          send_buff_.push_back(buf[i] + 1);
          event_ |= EVENT_WR;
        }
        break;
      }
    }
  }

  void on_handle_write() {
    if (send_buff_.empty()) {
      event_ |= EVENT_RD;
      event_ |= EVENT_WR;
      return;
    }

    auto [r, bytes] = s_.write_async(send_buff_.data(), send_buff_.size());
    if (!r) {
      event_ = EVENT_NO;
      return;
    }
    else {
      event_ |= EVENT_WR;
    }
    if (bytes <= 0)
      return;

    if (bytes < send_buff_.size()) {
      send_buff_.erase(send_buff_.begin(), send_buff_.begin() + bytes);
      event_ |= EVENT_WR;
    }
    else {
      send_buff_.clear();
      if (state_ == State::INIT_ACK)
        state_ = State::WAIT_MSG;
      event_ |= EVENT_RD;
    }
  }
};
using ConnPtr = std::shared_ptr<Conn>;

class PollEvent : private common::UnCopyable {
  std::list<ConnPtr> conns_;

  fd_set rfds_master_;
  fd_set wfds_master_;
public:
  PollEvent() noexcept {
    FD_ZERO(&rfds_master_);
    FD_ZERO(&wfds_master_);
  }

  ~PollEvent() {
    for (auto& c : conns_)
      c->get_socket().close();
  }

  void set_conn_event(ConnPtr& conn, int ev) {
    auto fd = conn->get_socket().get();
    if (ev | EVENT_RD) {
      FD_SET(fd, &rfds_master_);
      conn->set_event(EVENT_RD);
    }
    else {
      FD_CLR(fd, &rfds_master_);
      conn->cls_event(EVENT_RD);
    }

    if (ev | EVENT_WR) {
      FD_SET(fd, &wfds_master_);
      conn->set_event(EVENT_WR);
    }
    else {
      FD_CLR(fd, &wfds_master_);
      conn->cls_event(EVENT_WR);
    }
  }

  void append_conn(SOCKET fd, int ev, bool is_new = true) {
    auto conn = std::make_shared<Conn>(fd);
    conn->on_handle_connected();
    set_conn_event(conn, ev);
    conns_.push_back(conn);
  }

  void poll(SOCKET listen_fd) {
    fd_set rfds = rfds_master_;
    fd_set wfds = wfds_master_;
    int nready = ::select(-1, &rfds, &wfds, nullptr, nullptr);
    if (nready < 0)
      return;

    for (auto it = conns_.begin(); it != conns_.end();) {
      if (nready <= 0)
        break;

      auto& conn = *it;
      auto fd = conn->get_socket().get();

      bool need_erase = false;
      bool has_sloved = false;
      if (FD_ISSET(fd, &rfds)) {
        if (fd == listen_fd) {
          auto [r, s] = conn->get_socket().accept_async();
          if (!r)
            return;

          append_conn(s.get(), EVENT_RD | EVENT_WR);
        }
        else {
          conn->on_handle_read();
          set_conn_event(conn, conn->get_event());
          if (conn->is_nevent())
            need_erase = true;
        }
        has_sloved = true;
      }

      if (FD_ISSET(fd, &wfds)) {
        conn->on_handle_write();
        set_conn_event(conn, conn->get_event());
        if (conn->is_nevent())
          need_erase = true;

        has_sloved = true;
      }

      if (has_sloved)
        --nready;

      if (need_erase)
        conns_.erase(it++);
      else
        ++it;
    }
  }
};

void launch_server() {
  common::UniqueSocket listen_sockfd;
  if (auto fd = svrutils::create_server(); !fd)
    return;
  else
    listen_sockfd.reset(*fd);

  PollEvent pev;
  pev.append_conn(listen_sockfd, EVENT_RD, false);

  for (;;) {
    pev.poll(listen_sockfd);
  }
}

}

EFW_EXAMPLE(SelectServer, ss3) {
  common::WSGuarder guard;

  ss::launch_server();
}