// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_NET_TCPCONNECTION_HEADER_H__
#define __TYR_NET_TCPCONNECTION_HEADER_H__

#include <memory>
#include <string>
#include "../basic/TUnCopyable.h"
#include "../basic/TTimestamp.h"
#include "TBuffer.h"
#include "TCallbacks.h"
#include "TInetAddress.h"

namespace tyr { namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection
  : private basic::UnCopyable
  , public std::enable_shared_from_this<TcpConnection> {
  enum State {
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_DISCONNECTING,
    STATE_DISCONNECTED,
  };

  EventLoop* loop_{};
  std::string name_;
  State state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;
  ConnectionCallback connection_fn_;
  MessageCallback message_fn_;
  CloseCallback close_fn_;
  Buffer input_buff_;
  Buffer output_buff_;

  void set_state(State s) {
    state_ = s;
  }

  void handle_read(basic::Timestamp recv_time);
  void handle_write(void);
  void handle_close(void);
  void handle_error(void);
  void write_in_loop(const std::string& message);
  void shutdown_in_loop(void);
public:
  TcpConnection(EventLoop* loop, const std::string& name,
      int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr);
  ~TcpConnection(void);

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
    return state_ == STATE_CONNECTED;
  }

  void set_connection_callback(const ConnectionCallback& fn) {
    connection_fn_ = fn;
  }

  void set_message_callback(const MessageCallback& fn) {
    message_fn_ = fn;
  }

  void set_close_callback(const CloseCallback& fn) {
    close_fn_ = fn;
  }

  void connect_established(void);
  void connect_destroyed(void);
  void write(const std::string& message);
  void shutdown(void);
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}}

#endif // __TYR_NET_TCPCONNECTION_HEADER_H__
