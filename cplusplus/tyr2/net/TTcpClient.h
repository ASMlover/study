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
#ifndef __TYR_NET_TCPCLIENT_HEADER_H__
#define __TYR_NET_TCPCLIENT_HEADER_H__

#include <memory>
#include <string>
#include "../basic/TUnCopyable.h"
#include "../basic/TMutex.h"
#include "TCallbacks.h"
#include "TTcpConnection.h"

namespace tyr { namespace net {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class EventLoop;
class InetAddress;

class TcpClient : private basic::UnCopyable {
  EventLoop* loop_{};
  ConnectorPtr connector_{};
  const std::string name_;
  ConnectionCallback connection_fn_{};
  MessageCallback message_fn_{};
  WriteCompleteCallback write_complete_fn_{};
  bool retry_{};
  bool connect_{true};
  int next_connid_{1};
  mutable basic::Mutex mtx_;
  TcpConnectionPtr connection_;

  void new_connection(int sockfd);
  void remove_connection(const TcpConnectionPtr& conn);
public:
  TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string& name);
  ~TcpClient(void);

  void connect(void);
  void disconnect(void);
  void stop(void);

  TcpConnectionPtr get_connection(void) const {
    basic::MutexGuard guard(mtx_);
    return connection_;
  }

  EventLoop* get_loop(void) const {
    return loop_;
  }

  bool get_retry(void) const {
    return retry_;
  }

  void enable_retry(void) {
    retry_ = true;
  }

  void set_connection_callback(const ConnectionCallback& fn) {
    connection_fn_ = fn;
  }

  void set_connection_callback(ConnectionCallback&& fn) {
    connection_fn_ = std::move(fn);
  }

  void set_message_callback(const MessageCallback& fn) {
    message_fn_ = fn;
  }

  void set_message_callback(MessageCallback&& fn) {
    message_fn_ = std::move(fn);
  }

  void set_write_complete_callback(const WriteCompleteCallback& fn) {
    write_complete_fn_ = fn;
  }

  void set_write_complete_callback(WriteCompleteCallback&& fn) {
    write_complete_fn_ = std::move(fn);
  }
};

}}

#endif // __TYR_NET_TCPCLIENT_HEADER_H__
