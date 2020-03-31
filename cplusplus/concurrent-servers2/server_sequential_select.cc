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

  inline bool is_nev() const noexcept { return ev_ == EVNO; }
  inline bool is_rev() const noexcept { return ev_ | EVRD; }
  inline bool is_wev() const noexcept { return ev_ | EVWR; }
  inline int get_ev() const noexcept { return ev_; }
  inline void set_ev(int ev) noexcept { ev_ |= ev; }
  inline void clr_ev(int ev) noexcept { ev_ &= ~ev; }

  void on_event_connected() {
    status_ = sequential::Status::INIT_ACK;
    ev_ = EVRD;
    send_buf_.clear();
    send_buf_.push_back('*');
  }

  void on_event_read() {
  }

  void on_event_write() {
  }
};
using ConnPtr = std::shared_ptr<Conn>;

class EventLoop final : private coext::UnCopyable {
  std::list<ConnPtr> conns_;

  fd_set rfds_master_;
  fd_set wfds_master_;
public:
  EventLoop() noexcept {
    FD_ZERO(&rfds_master_);
    FD_ZERO(&wfds_master_);
  }
};

void launch_server() {
  std::unique_ptr<coext::Socket, std::function<void (coext::Socket*)>> svr{
    new coext::Socket{}, [](coext::Socket* s) { s->close(); }
  };

  if (!svr->open() || !svr->bind("0.0.0.0") || !svr->listen())
    return;
}

}

COEXT_EXAMPLE(SequentialServerSelect, sss1, "A sequential server with select") {
  coext::WSGuarder guard;

  sss1::launch_server();
}