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
#ifndef NEPTUNE_TCPCLIENT_H
#define NEPTUNE_TCPCLIENT_H

#include <memory>
#include <string>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Neptune/Callbacks.h>

namespace Neptune {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class EventLoop;
class InetAddress;

class TcpClient : private Chaos::UnCopyable {
  EventLoop* loop_{};
  ConnectorPtr connector_{};
  const std::string name_;
  Neptune::ConnectionCallback connection_fn_{};
  Neptune::MessageCallback message_fn_{};
  Neptune::WriteCompleteCallback write_complete_fn_{};
  bool retry_{};
  bool need_connect_{true};
  int next_connid_{1};
  mutable Chaos::Mutex mutex_;
  TcpConnectionPtr connection_;

  void do_handle_new_connection(int sockfd);
  void remove_connection(const TcpConnectionPtr& conn);
public:
  TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string& name);
  ~TcpClient(void);

  void connect(void);
  void disconnect(void);
  void stop(void);

  TcpConnectionPtr get_connection(void) const {
    Chaos::ScopedLock<Chaos::Mutex> guard(mutex_);
    return connection_;
  }

  EventLoop* get_loop(void) const {
    return loop_;
  }

  bool get_retry(void) const {
    return retry_;
  }

  void enabled_retry(void) {
    retry_ = true;
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
};

}

#endif // NEPTUNE_TCPCLIENT_H
