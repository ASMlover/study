#include <iostream>
#include "net.hh"
#include "examples.hh"

namespace echo::client {

void launch_client() {
  sel::net::Socket sock;

  sock.open();
  sock.connect();

  sock.write(sel::net::buffer("[ECHO]: Hello, world!"));

  char rbuf[128]{};
  sock.read(sel::net::buffer(rbuf, sizeof(rbuf)));

  std::cout << "ECHO BACK: " << rbuf << std::endl;

  sock.close();
}

}

SEL_EXAMPLE(Client, ec, "client example for `selectors`") {
  sel::net::Initializer<> init;
  echo::client::launch_client();
}