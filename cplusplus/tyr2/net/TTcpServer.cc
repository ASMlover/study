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
#include <stdio.h>
#include <functional>
#include "../basic/TLogging.h"
#include "TAcceptor.h"
#include "TEventLoop.h"
#include "TEventLoopThreadPool.h"
#include "TTcpConnection.h"
#include "TInetAddress.h"
#include "TTcpServer.h"

namespace tyr { namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& host_addr, const std::string& name, Option opt)
  : loop_(TCHECK_NOTNULL(loop))
  , ip_port_(host_addr.to_host_port())
  , name_(name)
  , acceptor_(new Acceptor(loop_, host_addr))
  , thread_pool_(new EventLoopThreadPool(loop_, name_))
  , connection_fn_(default_connection_callback)
  , message_fn_(default_message_callback) {
  acceptor_->set_new_connection_callback(
      std::bind(&TcpServer::new_connection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer(void) {
  loop_->assert_in_loopthread();
  TYRLOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& conn_obj : connections_) {
    TcpConnectionPtr conn = conn_obj.second;
    conn_obj.second.reset();
    conn->get_loop()->run_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
    conn.reset();
  }
}

void TcpServer::start(void) {
  if (0 == started_++) {
    thread_pool_->start(thread_init_fn_);

    assert(!acceptor_->is_listenning());
    if (!acceptor_->is_listenning())
      loop_->run_in_loop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

void TcpServer::set_thread_count(int thread_count) {
  assert(0 <= thread_count);
  thread_pool_->set_thread_count(thread_count);
}

void TcpServer::new_connection(int sockfd, const InetAddress& peeraddr) {
  loop_->assert_in_loopthread();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_connid_);
  ++next_connid_;
  std::string conn_name = name_ + buf;

  TYRLOG_INFO << "TcpServer::new_connection [" << name_
    << "] - new connection [" << conn_name
    << "] from " << peeraddr.to_host_port();
  InetAddress localaddr(SocketSupport::kern_localaddr(sockfd));

  EventLoop* io_loop = thread_pool_->get_next_loop();
  TcpConnectionPtr conn(new TcpConnection(io_loop, conn_name, sockfd, localaddr, peeraddr));
  connections_[conn_name] = conn;
  conn->set_connection_callback(connection_fn_);
  conn->set_message_callback(message_fn_);
  conn->set_write_complete_callback(write_complete_fn_);
  conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
  io_loop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
  loop_->run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr& conn) {
  loop_->assert_in_loopthread();
  TYRLOG_INFO << "TcpServer::remove_connection [" << name_
    << "] - connection " << conn->get_name();
  size_t n = connections_.erase(conn->get_name());
  UNUSED(n);
  EventLoop* io_loop = conn->get_loop();
  io_loop->put_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}

}}
