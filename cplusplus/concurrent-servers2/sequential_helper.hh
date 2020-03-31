#pragma once

#include "conet.hh"

namespace sequential {

enum class Status {
  INIT_ACK,
  WAIT_MSG,
  READ_MSG,
};

void serve_blocking_logic(coext::Socket& conn);

}