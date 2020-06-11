#include "net_socket.hh"
#include "net_selector.hh"

namespace sel::net {

inline int constexpr fdmax(int a, socket_t b) noexcept {
  return a > as_type<int>(b) ? a : as_type<int>(b);
}

Selector::Selector() noexcept {
  FD_ZERO(&rfds_master_);
  FD_ZERO(&wfds_master_);
}

Selector::~Selector() {
  for (auto& s : sockets_)
    s.second.first->close();
  sockets_.clear();
}

void Selector::reg(const SocketPtr& s, Event events, EventFn&& fn) {
  auto sockfd = s->sockfd();
  if (events & Event::READ)
    FD_SET(sockfd, &rfds_master_);
  if (events & Event::WRITE)
    FD_SET(sockfd, &wfds_master_);

  max_sockfd_ = fdmax(max_sockfd_, sockfd);
  if (auto it = sockets_.find(sockfd); it == sockets_.end())
    sockets_[sockfd] = std::make_pair(s, std::move(fn));
}

void Selector::unreg(const SocketPtr& s) {
  auto sockfd = s->sockfd();
  FD_CLR(sockfd, &rfds_master_);
  FD_CLR(sockfd, &wfds_master_);

  if (auto it = sockets_.find(sockfd); it != sockets_.end()) {
    s->close();
    sockets_.erase(it);
  }
}

std::vector<EventItem> Selector::select() {
  std::vector<EventItem> evs;

  fd_set rfds = rfds_master_;
  fd_set wfds = wfds_master_;
  int nready = ::select(max_sockfd_, &rfds, &wfds, nullptr, nullptr);
  if (nready < 0)
    return evs;

  for (auto& s : sockets_) {
    if (nready <= 0)
      break;

    auto sockfd = s.first;
    Event ev{ Event::NONE };
    if (FD_ISSET(sockfd, &rfds))
      ev |= Event::READ;
    if (FD_ISSET(sockfd, &wfds))
      ev |= Event::WRITE;

    if (ev != Event::NONE) {
      --nready;
      evs.push_back({ s.second.first, ev, s.second.second });
    }
  }

  return evs;
}

}