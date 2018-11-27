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

#include <core/NyxInternal.h>
#include <core/BaseServer.h>

namespace nyx {

class TcpListenSession;

class TcpServer : public BaseServer {
  using SessionPtr = std::shared_ptr<TcpListenSession>;

  tcp::acceptor acceptor_;
  std::string host_{};
  std::uint16_t port_{};
  bool reuse_addr_{};
  int backlog_{};
  SessionPtr new_conn_;
public:
  TcpServer(void);
  virtual ~TcpServer(void);

  virtual void invoke_launch(void) override;
  virtual void invoke_shutoff(void) override;

  void bind(const std::string& host, std::uint16_t port);
  void listen(int backlog);

  void enable_reuse_addr(bool reuse) {
    reuse_addr_ = reuse;
  }

  bool is_open(void) const {
    return acceptor_.is_open();
  }
private:
  void reset_connection(void);
  void handle_async_accept(const std::error_code& ec);
};

}
