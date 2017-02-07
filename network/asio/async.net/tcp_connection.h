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

#include <atomic>
#include <deque>
#include <vector>
#include "common.h"
#include "connection_base.h"

class TcpConnection : public ConnectionBase {
  tcp::socket socket_;
  bool disconnected_{};
  bool sending_{};
  std::atomic<bool> closed_{false};
  std::uint32_t data_queue_count_{};
  std::uint32_t nwrite_limit_{};
  std::vector<char> buffer_;
  std::deque<WriteBufPtr> write_queue_;

  void do_async_write_impl(WriteBufPtr buf) {
    if (nwrite_limit_ > 0 && data_queue_count_ > nwrite_limit_)
      return;

    if (sending_) {
      write_queue_.push_back(buf);
      data_queue_count_ += buf->size();
    }
    else {
      sending_ = true;
      handle_async_write(buf);
    }
  }

  void handle_async_write(WriteBufPtr buf) {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, *buf,
        [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
          if (!write_queue_.empty()) {
            auto buf = write_queue_.front();
            write_queue_.pop_front();
            data_queue_count_ -= buf->size();
            handle_async_write(buf);
          }
          else {
            sending_ = false;
            if (disconnected_ && do_stop())
              do_unregister();
          }
        });
  }
public:
  TcpConnection(boost::asio::io_service& io_service);
  virtual ~TcpConnection(void) override;

  virtual void do_start(void) override;
  virtual bool do_stop(void) override;
  virtual void do_disconnect(void) override;
  virtual const std::string get_remote_addr(void) const override;
  virtual std::int16_t get_remote_port(void) const override;
  const std::string get_local_addr(void) const;
  std::int16_t get_local_port(void) const;

  tcp::socket& get_socket(void) {
    return socket_;
  }

  void set_buffer_size(std::size_t size) {
    std::vector<char>(size).swap(buffer_);
  }

  std::size_t get_buffer_size(void) const {
    return buffer_.size();
  }

  void set_write_limit(std::uint32_t limit) {
    nwrite_limit_ = limit;
  }

  std::uint32_t get_write_limit(void) const {
    return nwrite_limit_;
  }

  std::uint32_t get_data_queue_count(void) const {
    return data_queue_count_;
  }

  virtual void do_async_write(WriteBufPtr buf) override {
    do_async_write_impl(buf);
  }

  virtual void do_async_write(WriteBufPtr buf, bool /*reliable*/, std::int8_t /*channel*/) override {
    do_async_write_impl(buf);
  }
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
