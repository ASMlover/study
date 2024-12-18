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
#include <deque>
#include <vector>
#include <core/NyxInternal.hh>
#include <core/net/BaseSession.hh>

namespace nyx::net {

class TcpSession : public BaseSession {
protected:
  tcp::socket socket_;
  bool is_disconnected_{};
  bool is_writing_{};
  std::atomic<bool> is_closed_{false};
  std::uint32_t data_queue_size_{};
  std::uint32_t nwrite_limit_{};
  std::vector<char> buffer_;
  std::deque<std::string> write_queue_;
public:
  TcpSession(boost::asio::io_context& context) noexcept;
  virtual ~TcpSession(void);

  tcp::socket& get_socket(void) noexcept {
    return socket_;
  }

  bool is_closed(void) const noexcept {
    return is_closed_.load();
  }

  void set_buffer_size(std::size_t len) noexcept {
    std::vector<char>(len).swap(buffer_);
  }

  std::size_t get_buffer_size(void) const noexcept {
    return buffer_.size();
  }

  void set_write_limit(std::uint32_t limit) noexcept {
    nwrite_limit_ = limit;
  }

  std::uint32_t get_write_limit(void) const noexcept {
    return nwrite_limit_;
  }

  std::uint32_t get_data_queue_size(void) const noexcept {
    return data_queue_size_;
  }

  const std::string get_local_host(void) const {
    return socket_.lowest_layer().local_endpoint().address().to_string();
  }

  std::uint16_t get_local_port(void) const {
    return socket_.lowest_layer().local_endpoint().port();
  }

  virtual const std::string get_remote_host(void) const override {
    return socket_.lowest_layer().remote_endpoint().address().to_string();
  }

  virtual std::uint16_t get_remote_port(void) const override {
    return socket_.lowest_layer().remote_endpoint().port();
  }
protected:
  virtual void cleanup(void);
  virtual void invoke_launch(void) override;
  virtual bool invoke_shutoff(void) override;
  virtual void invoke_disconnect(void) override;
  virtual void invoke_async_write(const std::string& buf) override;
  virtual void handle_async_read(const std::error_code& ec, std::size_t n);
private:
  void do_async_write_impl(const std::string& buf);
  void handle_async_write(const std::string& buf);
};

}
