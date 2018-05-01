// Copyright (c) 2018 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "nyx_server.h"

namespace nyx { namespace net {

class tcp_listen_connection;
using tcp_listen_connection_ptr = std::shared_ptr<tcp_listen_connection>;

class tcp_server : public server {
  boost::asio::ip::tcp::acceptor acceptor_;
  std::size_t backlog_{};
  std::string host_{};
  std::uint16_t port_{};
  bool reuse_addr_{};
  tcp_listen_connection_ptr new_connection_;
public:
  tcp_server(void);
  virtual ~tcp_server(void);

  virtual void start(void) override;
  virtual void stop(void) override;

  void bind(const std::string& host, std::uint16_t port);
  void listen(std::size_t backlog);
private:
  void handle_start_server(void);
  virtual void reset_connection(void);
  virtual void handle_accept(const boost::system::error_code& ec);
};

}}
