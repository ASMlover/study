#include <unordered_map>
#include "common_utils.hh"
#include "server_utils.hh"
#include "examples.hh"

namespace ss {

static constexpr int EVENT_NO = 0; // no event
static constexpr int EVENT_RD = 1; // read event
static constexpr int EVENT_WR = 2; // write event

class Connector : private common::UnCopyable {
  enum class State {
    INIT_ACK,
    WAIT_MSG,
    READ_MSG,
  };

  SOCKET sockfd_{};
  State state_{ State::INIT_ACK };
  int event_{ EVENT_NO };
  std::vector<char> send_buff_;
public:
  Connector(SOCKET fd) noexcept
    : sockfd_(fd) {
  }

  inline void add_event(int ev) noexcept { event_ = event_ | ev; }
  inline void del_event(int ev) noexcept { event_ = event_ & ~(ev); }
  inline SOCKET get_fd() const { return sockfd_; }

  void close() {
    shutdown(sockfd_, SD_BOTH);
    closesocket(sockfd_);
  }

  int handle_connected() {
    state_ = State::INIT_ACK;
    send_buff_.push_back('*');
    event_ = EVENT_WR;

    return EVENT_WR;
  }

  int handle_revent() {
    if (state_ == State::INIT_ACK || !send_buff_.empty())
      return EVENT_WR;

    char buf[1024];
    int nbytes = recv(sockfd_, buf, sizeof(buf), 0);
    if (nbytes == 0) {
      return EVENT_NO;
    }
    else if (nbytes < 0) {
      auto err = WSAGetLastError();
      if (err == WSAEWOULDBLOCK)
        return EVENT_RD;
      else
        return EVENT_NO;
    }

    bool ready_to_send = false;
    for (int i = 0; i < nbytes; ++i) {
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
          ready_to_send = true;
        }
        break;
      }
    }

    return (!ready_to_send ? EVENT_RD : EVENT_NO) | (ready_to_send ? EVENT_WR : EVENT_NO);
  }

  int handle_wevent() {
    if (send_buff_.empty())
      return EVENT_RD | EVENT_WR;

    int nbytes = send(sockfd_, send_buff_.data(), static_cast<int>(send_buff_.size()), 0);
    if (nbytes == -1) {
      auto err = WSAGetLastError();
      if (err == WSAEWOULDBLOCK)
        return EVENT_WR;
      else
        return EVENT_NO;
    }

    if (nbytes < static_cast<int>(send_buff_.size())) {
      send_buff_.erase(send_buff_.begin(), send_buff_.begin() + nbytes);
      send_buff_.shrink_to_fit();
      return EVENT_WR;
    }
    else {
      send_buff_.clear();
      if (state_ == State::INIT_ACK)
        state_ = State::WAIT_MSG;

      return EVENT_RD;
    }
  }
};

void launch_server() {
  std::unordered_map<SOCKET, std::shared_ptr<Connector>> conns;

  auto append_conn = [&conns](SOCKET fd, fd_set* set, int ev) {
    FD_SET(fd, set);

    auto conn = std::make_shared<Connector>(fd);
    conn->add_event(ev);

    conns.insert(std::make_pair(fd, conn));
  };

  auto append_newconn = [&conns](SOCKET fd, fd_set* rset, fd_set* wset) {
    u_long flags = 1;
    ioctlsocket(fd, FIONBIO, &flags);
    auto conn = std::make_shared<Connector>(fd);
    int ev = conn->handle_connected();
    if (ev | EVENT_RD)
      FD_SET(fd, rset);
    else
      FD_CLR(fd, rset);
    if (ev | EVENT_WR)
      FD_SET(fd, wset);
    else
      FD_CLR(fd, wset);

    conns.insert(std::make_pair(fd, conn));
  };

  common::UniqueSocket listen_sockfd;
  if (auto fd = svrutils::create_server(); !fd)
    return;
  else
    listen_sockfd.reset(*fd);

  svrutils::set_nonblocking(listen_sockfd);

  fd_set rfds_master;
  FD_ZERO(&rfds_master);
  fd_set wfds_master;
  FD_ZERO(&wfds_master);
  append_conn(listen_sockfd.get(), &rfds_master, EVENT_RD);

  for (;;) {
    fd_set rfds = rfds_master;
    fd_set wfds = wfds_master;

    int nready = select(-1, &rfds, &wfds, nullptr, nullptr);
    if (nready < 0)
      return;

    std::vector<SOCKET> fds;
    for (auto& c : conns)
      fds.push_back(c.first);

    for (auto& fd : fds) {
      if (nready <= 0)
        break;
      auto it = conns.find(fd);
      if (it == conns.end())
        break;

      bool need_erase = false;
      if (FD_ISSET(fd, &rfds_master)) {
        --nready;

        if (fd == listen_sockfd.get()) {
          sockaddr_in peer_addr;
          int peer_addr_len = static_cast<int>(sizeof(peer_addr));

          SOCKET new_sockfd = accept(listen_sockfd, (sockaddr*)&peer_addr, &peer_addr_len);
          if (new_sockfd == INVALID_SOCKET) {
            auto err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
              return;
          }
          else {
            append_newconn(new_sockfd, &rfds_master, &wfds_master);
          }
        }
        else {
          int ev = it->second->handle_revent();
          if (ev | EVENT_RD)
            FD_SET(fd, &rfds_master);
          else
            FD_CLR(fd, &rfds_master);
          if (ev | EVENT_WR)
            FD_SET(fd, &wfds_master);
          else
            FD_CLR(fd, &wfds_master);

          if (ev == EVENT_NO)
            need_erase = true;
        }
      }

      if (FD_ISSET(fd, &wfds_master)) {
        --nready;

        int ev = it->second->handle_wevent();
        if (ev | EVENT_RD)
          FD_SET(fd, &rfds_master);
        else
          FD_CLR(fd, &rfds_master);
        if (ev | EVENT_WR)
          FD_SET(fd, &wfds_master);
        else
          FD_CLR(fd, &wfds_master);

        if (ev == EVENT_NO)
          need_erase = true;
      }

      if (need_erase)
        conns.erase(it);
    }
  }
}

}

EFW_EXAMPLE(SelectServer, ss3) {
  common::WSGuarder guard;

  ss::launch_server();
}