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
#include <vector>
#include <boost/asio.hpp>
#include "nyx_connection.h"

namespace nyx { namespace net {

using async_writbuf_vec = std::vector<boost::asio::const_buffer>;

class tcp_listen_connection : public connection {
  boost::asio::ip::tcp::socket socket_; // connection socket
  boost::asio::ip::tcp::endpoint remote_ep_; // remote end point
  bool disconnected_{};
  bool is_sending_{};
  std::atomic<bool> closed_{};
  std::uint32_t data_queue_size_{};
  std::uint32_t send_limit_size_{};
  std::vector<char> buffer_;
  writbuf_ptr data_queue_;
  writbuf_ptr send_queue_;
public:
  tcp_listen_connection(boost::asio::io_service& io_service);
  virtual ~tcp_listen_connection(void);

  inline boost::asio::ip::tcp::socket& get_socket(void) {
    return socket_;
  }

  inline void set_buffer_size(std::size_t size) {
    std::vector<char> tmp(size);
    buffer_.swap(tmp);
  }

  inline std::size_t get_buffer_size(void) const {
    return buffer_.size();
  }

  inline void set_send_limit(std::uint32_t limit_size) {
    send_limit_size_ = limit_size;
  }

  inline std::uint32_t get_send_limit(void) const {
    return send_limit_size_;
  }

  inline std::uint32_t get_queue_size(void) const {
    return data_queue_size_;
  }

  std::string remote_endpoint_as_string(void) const {
    std::stringstream ss;
    ss << "tcp:" << remote_ep_;
    return ss.str();
  }

  virtual void do_start(void) override;
  virtual bool do_stop(void) override;
  virtual bool do_disconnect(void) override;
  virtual void do_async_write(const writbuf_ptr& buf) override;
  virtual void do_async_write(
      const writbuf_ptr& buf, bool reliable, std::uint8_t channel) override;
  virtual std::string remote_ip(void) const override;
  virtual std::uint16_t remote_port(void) const override;
protected:
  virtual void cleanup(void);
  virtual void handle_close_socket(void);
  virtual void handle_connection_read(
      const boost::system::error_code& ec, std::size_t bytes);
  virtual void handle_async_write(const boost::system::error_code& ec);

  void _do_async_write_impl(const writbuf_ptr& buf);
};

}}
