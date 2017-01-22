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
#ifndef NEPTUNE_TCPCONNECTION_H
#define NEPTUNE_TCPCONNECTION_H

#include <memory>
#include <string>
#include <Chaos/UnCopyable.h>
#include <Chaos/Container/StringPiece.h>
#include <Chaos/Datetime/Timestamp.h>
#include <Chaos/Utility/Any.h>
#include <Neptune/Buffer.h>
#include <Neptune/Callbacks.h>
#include <Neptune/InetAddress.h>

namespace Neptune {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : private Chaos::UnCopyable , public std::enable_shared_from_this<TcpConnection> {
  enum class NetLink {
    NETLINK_CONNECTING,
    NETLINK_CONNECTED,
    NETLINK_DISCONNECTING,
    NETLINK_DISCONNECTED,
  };

  EventLoop* loop_{};
  std::string name_;
  NetLink linkstate_{};
  bool reading_{};
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;
  Neptune::ConnectionCallback connection_fn_{};
  Neptune::MessageCallback message_fn_{};
  Neptune::WriteCompleteCallback write_complete_fn_{};
  Neptune::HighWaterMarkCallback high_watermark_fn_{};
  Neptune::CloseCallback close_fn_{};
  std::size_t high_watermark_{};
  Neptune::Buffer readbuff_;
  Neptune::Buffer writbuff_;
  Chaos::Any context_;

  void do_handle_read(Chaos::Timestamp recvtime);
  void do_handle_write(void);
  void do_handle_close(void);
  void do_handle_error(void);
  void write_in_loop(const Chaos::StringPiece& message);
  void write_in_loop(const void* buf, std::size_t len);
  const char* linkstate_to_string(void) const;

  void set_linkstate(NetLink linkstate) {
    linkstate_ = linkstate;
  }
public:
  TcpConnection(EventLoop* loop, const std::string& name,
      int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr);
  ~TcpConnection(void);

  void do_connect_established(void);
  void do_connect_destroyed(void);
  void write(const Chaos::StringPiece& message);
  void write(Buffer* buf);
  void write(void* buf, std::size_t len);
  void shutdown(void);
  void force_close(void);
  void force_close_with_delay(double seconds);
  void set_tcp_nodelay(bool nodelay = true);
  void start_read(void);
  void stop_read(void);

  EventLoop* get_loop(void) const {
    return loop_;
  }

  const std::string& get_name(void) const {
    return name_;
  }

  const InetAddress& get_local_address(void) const {
    return local_addr_;
  }

  const InetAddress& get_peer_address(void) const {
    return peer_addr_;
  }

  bool is_connected(void) const {
    return linkstate_ == NetLink::NETLINK_CONNECTED;
  }

  bool is_disconnected(void) const {
    return linkstate_ == NetLink::NETLINK_DISCONNECTED;
  }

  void bind_connection_functor(const Neptune::ConnectionCallback& fn) {
    connection_fn_ = fn;
  }

  void bind_connection_functor(Neptune::ConnectionCallback&& fn) {
    connection_fn_ = std::move(fn);
  }

  void bind_message_functor(const Neptune::MessageCallback& fn) {
    message_fn_ = fn;
  }

  void bind_message_functor(Neptune::MessageCallback&& fn) {
    message_fn_ = std::move(fn);
  }

  void bind_write_complete_functor(const Neptune::WriteCompleteCallback& fn) {
    write_complete_fn_ = fn;
  }

  void bind_write_complete_functor(Neptune::WriteCompleteCallback&& fn) {
    write_complete_fn_ = std::move(fn);
  }

  void bind_high_watermark_functor(const Neptune::HighWaterMarkCallback& fn, std::size_t watermark) {
    high_watermark_fn_ = fn;
    high_watermark_ = watermark;
  }

  void bind_high_watermark_functor(Neptune::HighWaterMarkCallback&& fn, std::size_t watermark) {
    high_watermark_fn_ = std::move(fn);
    high_watermark_ = watermark;
  }

  void bind_close_functor(const Neptune::CloseCallback& fn) {
    close_fn_ = fn;
  }

  void bind_close_functor(Neptune::CloseCallback&& fn) {
    close_fn_ = std::move(fn);
  }

  bool is_reading(void) const {
    return reading_;
  }

  void set_context(const Chaos::Any& context) {
    context_ = context;
  }

  const Chaos::Any& get_context(void) const {
    return context_;
  }

  Chaos::Any* get_mutable_context(void) {
    return &context_;
  }

  Buffer* get_readbuffer(void) {
    return &readbuff_;
  }

  Buffer* get_writbuffer(void) {
    return &writbuff_;
  }
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}

#endif // NEPTUNE_TCPCONNECTION_H
