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
#include "nyx_tcp_listen_connection.h"

namespace nyx { namespace net {

tcp_listen_connection::tcp_listen_connection(
    boost::asio::io_service& io_service)
  : connection(io_service)
  , socket_(io_service)
  , buffer_(1024)
  , data_queue_(new writbuf_vec())
  , send_queue_(new writbuf_vec()) {
}

tcp_listen_connection::~tcp_listen_connection(void) {
}

void tcp_listen_connection::do_start(void) {
  if (closed_.load())
    return;

  remote_ep_ = boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(remote_ip()), remote_port());

  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t n) {
        handle_connection_read(ec, n);
      });
}

bool tcp_listen_connection::do_stop(void) {
  bool false_flag{};
  if (!closed_.compare_exchange_strong(false_flag,
        true, std::memory_order_release, std::memory_order_relaxed))
    return false;

  auto self(shared_from_this());
  strand_->post([this, self] { handle_close_socket(); });
  return false;
}

void tcp_listen_connection::do_disconnect(void) {
  if (disconnected_)
    return;

  disconnected_ = true;
  if (!is_sending_) {
    if (do_stop())
      cleanup();
  }
}

std::string tcp_listen_connection::remote_ip(void) const {
  boost::system::error_code ec;
  return socket_.lowest_layer().remote_endpoint(ec).address().to_string();
}

std::uint16_t tcp_listen_connection::remote_port(void) const {
  boost::system::error_code ec;
  return socket_.lowest_layer().remote_endpoint(ec).port();
}

void tcp_listen_connection::cleanup(void) {
  do_unregister();
}

void tcp_listen_connection::handle_close_socket(void) {
  if (socket_.lowest_layer().is_open()) {
    try {
      socket_.lowest_layer().shutdown(
          boost::asio::ip::tcp::socket::shutdown_both);
      socket_.lowest_layer().close();
    }
    catch (boost::system::system_error& e) {
      (void)e;
      // TODO: show ec
      // auto ec = e.code();
    }
  }
}

void tcp_listen_connection::handle_connection_read(
    const boost::system::error_code& ec, std::size_t bytes) {
  if (closed_.load())
    return;

  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buffer_),
      [this, self](const boost::system::error_code& ec, std::size_t n) {
        handle_connection_read(ec, n);
      });
}

void tcp_listen_connection::handle_async_write(
    const boost::system::error_code& ec) {
  if (!is_sending_) {
    // ERROR:
  }

  if (closed_.load())
    return;

  send_queue_->clear();
  auto size = data_queue_->size();
  if (size > 0) {
    data_queue_.swap(send_queue_);
    if (size > 1) {
      async_writbuf_vec buf_vec(size);
      for (auto& s : *send_queue_) {
        buf_vec.emplace_back(static_cast<boost::asio::const_buffer>(s->data()));
        data_queue_size_ -= s->size();
      }

      auto self(shared_from_this());
      boost::asio::async_write(socket_, buf_vec,
          [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
            handle_async_write(ec);
          });
    }
    else {
      writbuf_ptr& buf = send_queue_->at(0);
      data_queue_size_ -= buf->size();

      auto self(shared_from_this());
      boost::asio::async_write(socket_, *buf,
          [this, self](const boost::system::error_code& ec, std::size_t /*n*/) {
            handle_async_write(ec);
          });
    }

    return;
  }
  else {
    is_sending_ = false;
    if (disconnected_) {
      if (do_stop())
        cleanup();
    }
  }
}

}}
