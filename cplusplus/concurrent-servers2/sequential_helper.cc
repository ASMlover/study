#include "sequential_helper.hh"

namespace sequential {

void serve_blocking_logic(coext::Socket& conn) {
  if (auto [r, n] = conn.write("*", 1); !r || n != 1)
    return;

  Status status = Status::WAIT_MSG;
  for (;;) {
    char buf[1024];
    auto [r, n] = conn.read(buf, sizeof(buf));
    if (!r)
      break;

    for (coext::sz_t i = 0; i < n; ++i) {
      switch (status) {
      case Status::INIT_ACK: break;
      case Status::WAIT_MSG:
        if (buf[i] == '^')
          status = Status::READ_MSG;
        break;
      case Status::READ_MSG:
        if (buf[i] == '$') {
          status = Status::WAIT_MSG;
        }
        else {
          char c = buf[i] + 1;
          if (auto [r, _] = conn.write(&c, 1); !r)
            return;
        }
        break;
      }
    }
  }
}

}