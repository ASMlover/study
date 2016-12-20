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
#ifndef __TYR_NET_TCPSERVER_HEADER_H__
#define __TYR_NET_TCPSERVER_HEADER_H__

#include <map>
#include <memory>
#include <string>
#include "../basic/TUnCopyable.h"
#include "TCallbacks.h"

namespace tyr { namespace net {

class Acceptor;
class EventLoop;
class InetAddress;

class TcpServer : private basic::UnCopyable {
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  bool started_{};
  int next_connid_{1};
  EventLoop* loop_{}; // acceptor event loop
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  ConnectionCallback connection_fn_;
  MessageCallback message_fn_;
  ConnectionMap connections_;

  void new_connection(int sockfd, const InetAddress& peeraddr);
  void remove_connection(const TcpConnectionPtr& conn);
public:
  TcpServer(EventLoop* loop, const InetAddress& listen_addr);
  ~TcpServer(void) = default;

  void start(void);

  void set_connection_callback(const ConnectionCallback& fn) {
    connection_fn_ = fn;
  }

  void set_message_callback(const MessageCallback& fn) {
    message_fn_ = fn;
  }
};

}}

#endif // __TYR_NET_TCPSERVER_HEADER_H__
