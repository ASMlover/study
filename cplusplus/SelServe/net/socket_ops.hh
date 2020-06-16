#pragma once

#include "types.hh"
#include "error.hh"

namespace sser::net::ops {

socket_t open(int af, int type, int protocal, std::error_code& ec);
int shutdown(socket_t sockfd, int how, std::error_code& ec);
int close(socket_t sockfd, std::error_code& ec);

bool set_non_blocking(socket_t sockfd, bool non_blocking, std::error_code& ec);

int bind(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec);
int listen(socket_t sockfd, int backlog, std::error_code& ec);

socket_t accept(socket_t sockfd, std::error_code& ec);
int connect(socket_t sockfd, strv_t host, u16_t port, std::error_code& ec);

}