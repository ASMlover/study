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

#include <cstdint>
#include <functional>
#include <memory>
#include "Nyx.h"

namespace nyx {

class BaseSession
  : private UnCopyable, public std::enable_shared_from_this<BaseSession> {
protected:
  asio::io_context::strand strand_;
  std::string local_addr_{};
  std::uint16_t local_port_{};
public:
  explicit BaseSession(asio::io_context& context);
  virtual ~BaseSession(void);

  bool is_alive(void);
  void register_session(void);
  void unregister_session(void);

  virtual const std::string get_remote_addr(void) const = 0;
  virtual std::uint16_t get_remote_port(void) const = 0;

  void launch(void) {
    invoke_launch();
  }

  void shutoff(void) {
    invoke_shutoff();
  }

  void disconnect(void) {
    auto self(shared_from_this());
    strand_.post([this, self] { invoke_disconnect(); });
  }

  void async_write(const std::string& buf) {
    auto self(shared_from_this());
    strand_.post([this, self, buf] { invoke_async_write(buf); });
  }

  void async_write(const char* buf, std::size_t len) {
    async_write(std::string(buf, len));
  }

  void set_local_endpoint(const std::string& addr, std::uint16_t port) {
    local_addr_ = addr;
    local_port_ = port;
  }

  void set_local_addr(const std::string& addr) {
    local_addr_ = addr;
  }

  void set_local_port(std::uint16_t port) {
    local_port_ = port;
  }

  const std::string& get_local_addr(void) const {
    return local_addr_;
  }

  std::uint16_t get_local_port(void) const {
    return local_port_;
  }
private:
  virtual void invoke_launch(void) = 0;
  virtual bool invoke_shutoff(void) = 0;
  virtual void invoke_disconnect(void) = 0;
  virtual void invoke_async_write(const std::string& buf) = 0;
};

}
