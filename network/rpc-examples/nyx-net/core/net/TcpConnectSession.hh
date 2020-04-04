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

#include <atomic>
#include <core/NyxInternal.hh>
#include <core/net/TcpSession.hh>

namespace nyx::net {

class TcpConnectSession : public TcpSession {
  using HandlerPtr = std::shared_ptr<CallbackHandler>;

  std::atomic<bool> is_connected_{};
  std::string host_{};
  std::uint16_t port_{};
  boost::asio::ip::tcp::resolver resolver_;
  HandlerPtr handler_;
public:
  TcpConnectSession(boost::asio::io_context& context);
  virtual ~TcpConnectSession(void);

  void async_connect(const std::string& host, std::uint16_t port);
  void async_write(const std::string& buf);
  void set_option(void);
  void set_callback_handler(const HandlerPtr& handler);

  virtual void invoke_launch(void) override;
  virtual bool invoke_shutoff(void) override;
  virtual void cleanup(void) override;
private:
  void handle_async_connect(
      const std::error_code& ec, tcp::resolver::iterator epiter);
  void handle_async_read(const std::error_code& ec, std::size_t n);
};

}
