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
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

namespace nyx { namespace net {

using writbuf_ptr = std::shared_ptr<boost::asio::streambuf>;
using writbuf_vec = std::vector<writbuf_ptr>;
using writbuf_vec_ptr = std::shared_ptr<writbuf_vec>;
using strand_ptr = std::shared_ptr<boost::asio::io_service::strand>;

class connection
  : public std::enable_shared_from_this<connection>
  , private boost::noncopyable {
protected:
  strand_ptr strand_;
  std::string local_ip_;
  std::uint16_t local_port_{};
public:
  explicit connection(boost::asio::io_service& io_service);
  virtual ~connection(void);

  void start(void);
  void stop(void);
  void do_register(void);
  void do_unregister(void);
  bool is_alive(void);

  inline strand_ptr get_strand(void) {
    return strand_;
  }

  inline void bind_local_addr(const std::string& ip, std::uint16_t port) {
    local_ip_ = ip;
    local_port_ = port;
  }

  inline void bind_local_ip(const std::string& ip) {
    local_ip_ = ip;
  }

  inline void bind_local_port(std::uint16_t port) {
    local_port_ = port;
  }

  inline std::string get_local_ip(void) const {
    return local_ip_;
  }

  inline std::uint16_t get_local_port(void) const {
    return local_port_;
  }

  virtual void do_start(void) = 0;
  virtual bool do_stop(void) = 0;
  virtual std::string remote_ip(void) const = 0;
  virtual std::uint16_t remote_port(void) const = 0;

  inline void async_write(const writbuf_ptr& buf) {
    auto self(shared_from_this());
    strand_->post([this, self, buf] { do_async_write(buf); });
  }
  virtual void do_async_write(const writbuf_ptr& buf) = 0;

  inline void async_write(
      const writbuf_ptr& buf, bool reliable, std::uint8_t channel) {
    auto self(shared_from_this());
    strand_->post([this, self, buf, reliable, channel] {
          do_async_write(buf, reliable, channel);
        });
  }
  virtual void do_async_write(
      const writbuf_ptr& buf, bool reliable, std::uint8_t channel) = 0;

  inline void disconnect(void) {
    auto self(shared_from_this());
    strand_->post([this, self] { do_disconnect(); });
  }
  virtual void do_disconnect(void) = 0;
};

}}
