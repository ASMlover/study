#pragma once

#include <optional>
#include "common_utils.hh"

namespace svrutils {

std::optional<SOCKET> create_server(std::uint16_t port = 5555);
void set_nonblocking(common::UniqueSocket& sockfd);
void serve_connection(common::UniqueSocket& sockfd);

}