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
#ifndef NEPTUNE_TCPSERVER_H
#define NEPTUNE_TCPSERVER_H

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <Chaos/UnCopyable.h>
#include <Neptune/Callbacks.h>

namespace Neptune {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class InetAddress;

class TcpServer : private Chaos::UnCopyable {
  using InitiateFunction = std::function<void (EventLoop*)>;
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  int next_connid_{1};
  std::atomic<std::int32_t> started_{};
  EventLoop* loop_{}; // acceptor event loop
  const std::string host_port_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> loop_threadpool_;
  Neptune::ConnectionCallback connection_fn_{};
  Neptune::MessageCallback message_fn_{};
  Neptune::WriteCompleteCallback write_complete_fn_{};
  InitiateFunction initiate_fn_{};
  ConnectionMap connections_;

  void do_handle_new_connection(int sockfd, const InetAddress& peer_addr);
  void remove_connection(const TcpConnectionPtr& conn);
public:
  enum class Option {
    OPTION_UNREUSEPORT,
    OPTION_REUSEPORT,
  };
  TcpServer(EventLoop* loop, const InetAddress& host_addr,
      const std::string& name, Option opt = Option::OPTION_UNREUSEPORT);
  ~TcpServer(void);

  void start(void);
  void set_nthreads(int nthreads);

  const std::string& get_host_port(void) const {
    return host_port_;
  }

  const std::string& get_name(void) const {
    return name_;
  }

  EventLoop* get_loop(void) const {
    return loop_;
  }

  std::shared_ptr<EventLoopThreadPool> get_loop_threadpool(void) {
    return loop_threadpool_;
  }

  void bind_initiate_functor(const InitiateFunction& fn) {
    initiate_fn_ = fn;
  }

  void bind_initiate_functor(InitiateFunction&& fn) {
    initiate_fn_ = std::move(fn);
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

#endif // NEPTUNE_TCPSERVER_H
