#include <iostream>
#include "net.hh"
#include "examples.hh"

namespace echo::server {

sel::net::Selector sel;

void on_read(sel::net::SocketPtr& sock, sel::net::Event ev) {
  char buf[1024]{};
  std::error_code ec;

  auto nread = sock->read_some(sel::net::buffer(buf), ec);
  if (ec) {
    sock->close();
    sel.unreg(sock);
    return;
  }

  sock->write_some(sel::net::buffer(static_cast<const char (&)[1024]>(buf), nread), ec);
}

void on_accept(sel::net::SocketPtr& sock, sel::net::Event ev) {
  std::error_code ec;
  auto new_sockfd = sock->accept(ec);
  if (ec) {
    sock->close();
    sel.unreg(sock);
    return;
  }

  auto new_conn = sel::net::make_socket(new_sockfd);
  new_conn->setblocking(false);
  sel.reg(new_conn, sel::net::Event::READ, on_read);
}

void launch_server() {
  auto sock = sel::net::make_socket();
  sock->open();
  sock->listen("0.0.0.0", 5555);
  sock->setblocking(false);

  sel.reg(sock, sel::net::Event::READ, on_accept);

  for (;;) {
    auto events = sel.select();
    for (auto& ev_item : events) {
      auto [s, ev, fn] = ev_item;
      fn(s, ev);
    }
  }
}

}

SEL_EXAMPLE(Server, es, "server example for `selectors`") {
  sel::net::Initializer<> init;
  echo::server::launch_server();
}