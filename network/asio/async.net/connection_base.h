// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <string>
#include <boost/noncopyable.hpp>
#include "common.h"

using WriteBufPtr = std::shared_ptr<boost::asio::streambuf>;

class ConnectionBase : private boost::noncopyable, public std::enable_shared_from_this<ConnectionBase> {
  StrandPtr strand_;
  std::string local_addr_;
  std::int16_t local_port_;
public:
  explicit ConnectionBase(boost::asio::io_service& io_service);
  virtual ~ConnectionBase(void);

  void start(void);
  virtual void do_start(void) = 0;
  void stop(void);
  virtual bool do_stop(void) = 0;
  bool is_alived(void);
  void do_register(void);
  void do_unregister(void);

  StrandPtr get_strand(void) {
    return strand_;
  }

  void disconnect(void) {
    auto self(shared_from_this());
    strand_->post([this, self] { do_disconnect(); });
  }
  virtual void do_disconnect(void) = 0;

  void async_write(WriteBufPtr buf) {
    auto self(shared_from_this());
    strand_->post([this, self, buf] { do_async_write(buf); });
  }
  virtual void do_async_write(WriteBufPtr /*buf*/) {}

  void async_write(WriteBufPtr buf, bool reliable, std::int8_t channel) {
    auto self(shared_from_this());
    strand_->post([this, self, buf, reliable, channel] {
          do_async_write(buf, reliable, channel);
        });
  }
  virtual void do_async_write(WriteBufPtr /*buf*/, bool /*reliable*/, std::int8_t /*channel*/) {}

  virtual const std::string& get_remote_addr(void) const = 0;
  virtual std::int16_t get_remote_port(void) const = 0;

  void bind_local_endpoint(const std::string& addr, std::int16_t port) {
    local_addr_ = addr;
    local_port_ = port;
  }

  void bind_local_addr(const std::string& addr) {
    local_addr_ = addr;
  }

  void bind_local_port(std::int16_t port) {
    local_port_ = port;
  }

  const std::string& get_local_addr(void) const {
    return local_addr_;
  }

  std::int16_t get_local_port(void) const {
    return local_port_;
  }
};

using ConnectionPtr = std::shared_ptr<ConnectionBase>;
